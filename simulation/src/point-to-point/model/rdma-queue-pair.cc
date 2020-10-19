
#include "rdma-queue-pair.h"

#include <ns3/assert.h>
#include <ns3/hash.h>
#include <ns3/ib-header.h>
#include <ns3/ipv4-header.h>
#include <ns3/last-packet-tag.h>
#include <ns3/log.h>
#include <ns3/rdma-hw.h>
#include <ns3/seq-ts-header.h>
#include <ns3/simulator.h>
#include <ns3/udp-header.h>
#include <ns3/uinteger.h>

#include <queue>

#include "ns3/ppp-header.h"
#include "ns3/rdma-app.h"

NS_LOG_COMPONENT_DEFINE("RdmaQueuePair");

namespace ns3 {

/**************************
 * RdmaQueuePair
 *************************/

void CongestionControlEntity::SetWin(uint32_t win) { m_win = win; }

void CongestionControlEntity::SetBaseRtt(uint64_t baseRtt) { m_baseRtt = baseRtt; }

void CongestionControlEntity::SetVarWin(bool v) { m_var_win = v; }

CongestionControlEntity::CongestionControlEntity() {
    m_win = 0;
    m_baseRtt = 0;
    m_max_rate = 0;
    m_var_win = false;
    m_rate = 0;

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

bool CongestionControlEntity::IsWinBound() {
    uint64_t w = GetWin();
    return w != 0 && GetOnTheFly() >= w;
}

uint64_t CongestionControlEntity::GetWin() {
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

uint64_t CongestionControlEntity::GetOnTheFly() { return 0; }
uint64_t CongestionControlEntity::GetBytesLeft() { return 0; }

uint64_t CongestionControlEntity::HpGetCurWin() {
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

uint64_t RdmaQueuePair::GetBytesLeft() { return m_size; }

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
    NS_LOG_FUNCTION(this);
    NS_LOG_LOGIC(" last_packet_seq: "
                 << " ack = " << std::dec << ack);
    if (ack > snd_una) {
        snd_una = ack;
    }
    if (GetQPType() == QPType::RDMA_RC) {
        // NS_ASSERT(!m_inflight_wrs.empty());
        while (!m_inflight_wrs.empty() && m_inflight_wrs.front()->last_packet_seq < ack) {
            Ptr<IBVWorkRequest> wr = m_inflight_wrs.front();
            m_inflight_wrs.pop();
            Ptr<IBVWorkCompletion> wc = Create<IBVWorkCompletion>(wr->tags.mark_tag_bits);
            wc->imm = wr->imm;
            wc->isTx = true;
            wc->qp = this;
            wc->size = wr->size;
            wc->tags = wr->tags;
            wc->completion_time_in_us = Simulator::Now().GetMicroSeconds();
            wc->verb = wr->verb;
            wc->wr = wr;
            NS_LOG_INFO(this << " posting wc, last_packet_seq = " << std::dec << wr->last_packet_seq << " ack = " << ack);
            m_notify_completion(wc);
        }
    }
}

// data path
Ptr<Packet> RdmaQueuePair::GetNextPacket() {
    NS_LOG_FUNCTION(this);
    if (m_sending_wr == nullptr) {
        if (!m_wrs.empty()) {
            m_sending_wr = m_wrs.front();
            m_wrs.pop();
            m_wr_remaining_size = m_sending_wr->size;
        } else {
            m_sending_wr = nullptr;
            Ptr<Packet> p = Create<Packet>(0);
            NS_LOG_ERROR("Create an zero byte pakcet when GetNextPacket");
            return p;
        }
    }
    uint32_t send_size = m_wr_remaining_size < m_rdma_hw->m_mtu ? m_wr_remaining_size : m_rdma_hw->m_mtu;
    NS_ASSERT(m_wr_remaining_size >= send_size);
    m_wr_remaining_size -= send_size;
    NS_ASSERT(m_size >= send_size);
    m_size -= send_size;
    IBHeader ibheader;
    ibheader.GetOpCode().SetOpCodeType(static_cast<OpCodeType>(m_connectionAttr.qp_type));

    if (m_sending_wr->size == send_size) {  // WR generate only one packet
        ibheader.GetOpCode().SetOpCodeOperation(OpCodeOperation::SEND_ONLY_WITH_IMM);
    } else if (m_wr_remaining_size == 0) {  // WR generate more than one packet, this is the last one
        ibheader.GetOpCode().SetOpCodeOperation(OpCodeOperation::SEND_LAST_WITH_IMM);
    } else if (m_wr_remaining_size != 0) {                            // WR generate more than one packet. this is not the last one
        if (send_size + m_wr_remaining_size == m_sending_wr->size) {  // WR generate more than one packet. this is the first one
            ibheader.GetOpCode().SetOpCodeOperation(OpCodeOperation::SEND_FIRST);
        } else {  // WR generate more than one packet. this is neither the last one nor the first one
            ibheader.GetOpCode().SetOpCodeOperation(OpCodeOperation::SEND_MIDDLE);
        }
    }

    Ptr<Packet> packet = Create<Packet>(send_size);

    if (LastPacketTag::HasLastPacketTag(ibheader.GetOpCode().GetOpCodeOperation())) {  // if this is the last packet, add Tags
        LastPacketTag last_pack_tag;
        last_pack_tag.SetIBV_WR(m_sending_wr);
        NS_LOG_LOGIC("sending last packet with tags: " << ((m_sending_wr->tags.mark_tag_bits == kGeneralTagPayloadBits) ? ("kGeneralTagPayloadBits")
                                                                                                                        : ("kLastTagPayloadBits")));
        packet->AddPacketTag(last_pack_tag);

        if (m_connectionAttr.qp_type == QPType::RDMA_UC) {
            Ptr<IBVWorkCompletion> wc = Create<IBVWorkCompletion>(m_sending_wr->tags.mark_tag_bits);
            wc->imm = m_sending_wr->imm;
            wc->isTx = true;
            wc->qp = this;
            wc->size = m_sending_wr->size;
            wc->tags = m_sending_wr->tags;
            wc->completion_time_in_us = Simulator::Now().GetMicroSeconds();
            wc->verb = m_sending_wr->verb;
            wc->wr = m_sending_wr;
            NS_LOG_LOGIC("successfully received a wr, posting wcs");
            m_notify_completion(wc);
        } else {
            m_sending_wr->last_packet_seq = snd_nxt;
            NS_LOG_LOGIC(this << " finished sending wr, waiting for acks, last_packet_seq = " << std::dec << m_sending_wr->last_packet_seq);
            m_inflight_wrs.push(m_sending_wr);
        }

        if (!m_wrs.empty()) {
            m_sending_wr = m_wrs.front();
            m_wrs.pop();
            m_wr_remaining_size = m_sending_wr->size;
        } else {
            m_sending_wr = nullptr;
        }
    } else {
        NS_LOG_LOGIC("sending packets without tags");
    }

    // add IBHeader
    packet->AddHeader(ibheader);

    // add SeqTsHeader
    SeqTsHeader seqTs;
    seqTs.SetSeq(snd_nxt);
    seqTs.SetPG(m_connectionAttr.pg);
    packet->AddHeader(seqTs);

    // add udp header
    UdpHeader udpHeader;
    udpHeader.SetDestinationPort(m_connectionAttr.dport);
    udpHeader.SetSourcePort(m_connectionAttr.sport);
    packet->AddHeader(udpHeader);
    // add ipv4 header
    Ipv4Header ipHeader;
    ipHeader.SetSource(m_connectionAttr.sip);
    ipHeader.SetDestination(m_connectionAttr.dip);
    ipHeader.SetProtocol(0x11);
    ipHeader.SetPayloadSize(packet->GetSize());
    ipHeader.SetTtl(64);
    ipHeader.SetTos(0);
    ipHeader.SetIdentification(m_ipid);
    packet->AddHeader(ipHeader);
    // add ppp header
    PppHeader ppp;
    ppp.SetProtocol(0x0021);  // EtherToPpp(0x800), see point-to-point-net-device.cc
    packet->AddHeader(ppp);

    // update state
    snd_nxt += send_size;
    m_ipid++;

    return packet;
}

bool RdmaQueuePair::GetNextIbvRequest_AssemblePacket_Finished(
    Ptr<Packet> packet, Ptr<IBVWorkRequest> &m_receiveWr) {  // The assembly and completion of m_receiveWr.The input packet is exactly what we want
    NS_ASSERT_MSG(packet != nullptr, "packet is NULL");

    IBHeader ibheader;
    packet->PeekHeader(ibheader);

    uint32_t payload_size = packet->GetZeroFilledSize();

    if (ibheader.GetOpCode().GetOpCodeOperation() == OpCodeOperation::SEND_FIRST) {
        m_receiveWr->size += payload_size;
    } else if (ibheader.GetOpCode().GetOpCodeOperation() == OpCodeOperation::SEND_MIDDLE) {
        m_receiveWr->size += payload_size;
    } else if (ibheader.GetOpCode().GetOpCodeOperation() == OpCodeOperation::SEND_LAST_WITH_IMM) {
        m_receiveWr->size += payload_size;
        return true;
    } else if (ibheader.GetOpCode().GetOpCodeOperation() == OpCodeOperation::SEND_ONLY_WITH_IMM) {
        m_receiveWr->size += payload_size;
        return true;
    }
    return false;
}

TypeId RdmaQueuePair::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::RdmaQueuePair").SetParent<Object>();
    return tid;
}

RdmaQueuePair::RdmaQueuePair(const QPConnectionAttr &attr) : m_connectionAttr(attr) {
    {
        ReceiverNextExpectedSeq = 0;
        m_nackTimer = Time(0);
        m_milestone_rx = 0;
        m_lastNACK = 0;
    }

    startTime = Simulator::Now();
    m_size = 0;
    snd_nxt = snd_una = 0;
    m_wr_remaining_size = 0;
    m_ipid = 0;
    m_nextAvail = Time(0);
    // m_rdma = appQp->get_rdma_driver()->m_rdma;
}

int RdmaQueuePair::ibv_post_send(Ptr<IBVWorkRequest> wr) {
    NS_LOG_FUNCTION(this);
    m_wrs.push(wr);
    m_size += wr->size;
    return 0;
}

void RdmaQueuePair::set_app_qp(Ptr<RdmaAppQP> app_qp) {
    m_app_qp = app_qp;
    m_notify_completion = MakeCallback(&RdmaAppQP::OnCompletion, m_app_qp);
}

void RdmaQueuePair::set_rdma_hw(Ptr<RdmaHw> rdma_hw) { m_rdma_hw = rdma_hw; }

/*********************
 * QueuePairSet
 ********************/
TypeId QueuePairSet::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::QueuePairSet").SetParent<Object>();
    return tid;
}

QueuePairSet::QueuePairSet(void) : mCCType(CongestionControlType::FlowBase){};

uint32_t QueuePairSet::GetN(void) { return m_qps.size(); }

Ptr<RdmaQueuePair> QueuePairSet::Get(uint32_t idx) { return m_qps[idx]; }

Ptr<RdmaQueuePair> QueuePairSet::operator[](uint32_t idx) { return m_qps[idx]; }

void QueuePairSet::AddQp(Ptr<RdmaQueuePair> qp) {
    m_qps.push_back(qp);

    /*
if (mCCType == CongestionControlType::FlowBase) {
    m_qps.push_back(qp);
} else {
    NS_ASSERT(mCCType == CongestionControlType::IPBase);

    for (int i = 0; i < m_qps.size(); i++) {
        Ptr<IPBasedCongestionControlEntity> ipCCSender = DynamicCast<IPBasedCongestionControlEntity,CongestionControlEntity>(m_qps[i]);
        if (qp->m_connectionAttr.sip == ipCCSender->mIPBasedFlow.sip && qp->m_connectionAttr.dip == ipCCSender->mIPBasedFlow.dip &&
            qp->m_connectionAttr.pg == ipCCSender->mIPBasedFlow.pg) {
            ipCCSender->m_QPs.push_back(qp);
            return;
        }
    }

    Ptr<IPBasedCongestionControlEntity> ipCCSender = Create<IPBasedCongestionControlEntity>();
    ipCCSender->mIPBasedFlow.sip = qp->m_connectionAttr.sip;
    ipCCSender->mIPBasedFlow.dip = qp->m_connectionAttr.dip;
    ipCCSender->mIPBasedFlow.pg = qp->m_connectionAttr.pg;
    ipCCSender->m_QPs.push_back(qp);
    m_qps.push_back(ipCCSender);
}
*/
}

void QueuePairSet::Clear(void) { m_qps.clear(); }

TypeId CongestionControlEntity::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::CongestionControlEntity").SetParent<Object>();
    return tid;
}

/*
TypeId IPBasedCongestionControlEntity::GetTypeId(void) {
static TypeId tid = TypeId("ns3::IPBasedCongestionControlEntity").SetParent<CongestionControlEntity>();
return tid;
}

void IPBasedCongestionControlEntity::SetIPBasedFlow(QPConnectionAttr& attr) {
mIPBasedFlow.sip = attr.sip;
mIPBasedFlow.dip = attr.dip;
mIPBasedFlow.pg = attr.pg;
};

*/
}  // namespace ns3
