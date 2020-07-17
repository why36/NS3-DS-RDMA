#include "rdma-queue-pair.h"

#include <ns3/hash.h>
#include <ns3/ipv4-header.h>
#include <ns3/seq-ts-header.h>
#include <ns3/simulator.h>
#include <ns3/udp-header.h>
#include <ns3/uinteger.h>

#include "ns3/ppp-header.h"

namespace ns3 {

/**************************
 * RdmaQueuePair
 *************************/
TypeId RdmaQueuePair::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::RdmaQueuePair").SetParent<CongestionControlSender>();
    return tid;
}

void CongestionControlSender::SetWin(uint32_t win) { m_win = win; }

void CongestionControlSender::SetBaseRtt(uint64_t baseRtt) { m_baseRtt = baseRtt; }

void CongestionControlSender::SetVarWin(bool v) { m_var_win = v; }

RdmaQueuePair::RdmaQueuePair(const QPConnectionAttr& attr) : m_connectionAttr(attr) {
    startTime = Simulator::Now();
    m_size = 0;
    snd_nxt = snd_una = 0;

    m_ipid = 0;
    m_win = 0;
    m_baseRtt = 0;
    m_max_rate = 0;
    m_var_win = false;
    m_rate = 0;
    m_nextAvail = Time(0);
    mlx.m_alpha = 1;
    mlx.m_alpha_cnp_arrived = false;
    mlx.m_first_cnp = true;
    mlx.m_decrease_cnp_arrived = false;
    mlx.m_rpTimeStage = 0;
    hp.m_lastUpdateSeq = 0;
    for (uint32_t i = 0; i < sizeof(hp.keep) / sizeof(hp.keep[0]); i++) hp.keep[i] = 0;
    hp.m_incStage = 0;
    hp.m_lastGap = 0;
    hp.u = 1;
    for (uint32_t i = 0; i < IntHeader::maxHop; i++) {
        hp.hopState[i].u = 1;
        hp.hopState[i].incStage = 0;
    }

    tmly.m_lastUpdateSeq = 0;
    tmly.m_incStage = 0;
    tmly.lastRtt = 0;
    tmly.rttDiff = 0;

    dctcp.m_lastUpdateSeq = 0;
    dctcp.m_caState = 0;
    dctcp.m_highSeq = 0;
    dctcp.m_alpha = 1;
    dctcp.m_ecnCnt = 0;
    dctcp.m_batchSizeOfAlpha = 0;

    hpccPint.m_lastUpdateSeq = 0;
    hpccPint.m_incStage = 0;
}

bool CongestionControlSender::IsWinBound() {
    uint64_t w = GetWin();
    return w != 0 && GetOnTheFly() >= w;
}

uint64_t CongestionControlSender::GetWin() {
    if (m_win == 0) return 0;
    uint64_t w;
    if (m_var_win) {
        w = m_win * m_rate.GetBitRate() / m_max_rate.GetBitRate();
        if (w == 0) w = 1;  // must > 0
    } else {
        w = m_win;
    }
    return w;
}

uint64_t CongestionControlSender::GetOnTheFly() { return 0; }
uint64_t CongestionControlSender::GetBytesLeft() { return 0; }
bool CongestionControlSender::IsFinished() { return true; }

uint64_t CongestionControlSender::HpGetCurWin() {
    if (m_win == 0) return 0;
    uint64_t w;
    if (m_var_win) {
        w = m_win * hp.m_curRate.GetBitRate() / m_max_rate.GetBitRate();
        if (w == 0) w = 1;  // must > 0
    } else {
        w = m_win;
    }
    return w;
}

void RdmaQueuePair::SetSize(uint64_t size) { m_size = size; }

void RdmaQueuePair::SetAppNotifyCallback(Callback<void> notifyAppFinish) { m_notifyAppFinish = notifyAppFinish; }

void RdmaQueuePair::SetCompletionCallback(Callback<void, IBVWorkCompletion&> notifyCompletion) { m_notifyCompletion = notifyCompletion; }

uint64_t RdmaQueuePair::GetBytesLeft() { return m_size >= snd_nxt ? m_size - snd_nxt : 0; }

Ptr<RdmaQueuePair> RdmaQueuePair::GetNextQp() { return this; };

uint32_t RdmaQueuePair::GetHash(void) {
    union {
        struct {
            uint32_t sip, dip;
            uint16_t sport, dport;
        };
        char c[12];
    } buf;
    buf.sip = m_connectionAttr.sip.Get();
    buf.dip = m_connectionAttr.dip.Get();
    buf.sport = m_connectionAttr.sport;
    buf.dport = m_connectionAttr.dport;
    return Hash32(buf.c, 12);
}

void RdmaQueuePair::Acknowledge(uint64_t ack) {
    if (ack > snd_una) {
        snd_una = ack;
    }
}

uint64_t RdmaQueuePair::GetOnTheFly() { return snd_nxt - snd_una; }

bool RdmaQueuePair::IsFinished() { return snd_una >= m_size; }

/*********************
 * RdmaRxQueuePair
 ********************/
TypeId CongestionControlReceiver::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::CongestionControlReceiver").SetParent<Object>();
    return tid;
}

TypeId RdmaRxQueuePair::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::RdmaRxQueuePair").SetParent<CongestionControlReceiver>();
    return tid;
}

RdmaRxQueuePair::RdmaRxQueuePair() {
    m_ipid = 0;
    ReceiverNextExpectedSeq = 0;
    m_nackTimer = Time(0);
    m_milestone_rx = 0;
    m_lastNACK = 0;
}

uint32_t RdmaRxQueuePair::GetHash(void) {
    union {
        struct {
            uint32_t sip, dip;
            uint16_t sport, dport;
        };
        char c[12];
    } buf;
    buf.sip = m_connectionAttr.sip.Get();
    buf.dip = m_connectionAttr.dip.Get();
    buf.sport = m_connectionAttr.sport;
    buf.dport = m_connectionAttr.dport;
    return Hash32(buf.c, 12);
}

/*********************
 * RdmaCongestionControlGroup
 ********************/
TypeId RdmaCongestionControlGroup::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::RdmaCongestionControlGroup").SetParent<Object>();
    return tid;
}

RdmaCongestionControlGroup::RdmaCongestionControlGroup(void) {}

uint32_t RdmaCongestionControlGroup::GetN(void) { return m_qps.size(); }

Ptr<CongestionControlSender> RdmaCongestionControlGroup::Get(uint32_t idx) { return m_qps[idx]; }

Ptr<CongestionControlSender> RdmaCongestionControlGroup::operator[](uint32_t idx) { return m_qps[idx]; }

void RdmaCongestionControlGroup::AddQp(Ptr<CongestionControlSender> qp) { m_qps.push_back(qp); }

#if 0
void RdmaCongestionControlGroup::AddRxQp(Ptr<RdmaRxQueuePair> rxQp){
	m_rxQps.push_back(rxQp);
}
#endif

void RdmaCongestionControlGroup::Clear(void) { m_qps.clear(); }

}  // namespace ns3
