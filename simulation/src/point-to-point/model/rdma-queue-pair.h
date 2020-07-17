#ifndef RDMA_QUEUE_PAIR_H
#define RDMA_QUEUE_PAIR_H

#include <ns3/custom-header.h>
#include <ns3/data-rate.h>
#include <ns3/event-id.h>
#include <ns3/int-header.h>
#include <ns3/ipv4-address.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/rdma.h>

#include <vector>

namespace ns3 {

namespace CongestionControl {
using DCQCN = struct dcqcn {
    DataRate m_targetRate;  //< Target rate
    EventId m_eventUpdateAlpha;
    double m_alpha;
    bool m_alpha_cnp_arrived;  // indicate if CNP arrived in the last slot
    bool m_first_cnp;          // indicate if the current CNP is the first CNP
    EventId m_eventDecreaseRate;
    bool m_decrease_cnp_arrived;  // indicate if CNP arrived in the last slot
    uint32_t m_rpTimeStage;
    EventId m_rpTimer;
};

using HPCC = struct hpcc {
    uint32_t m_lastUpdateSeq;
    DataRate m_curRate;
    IntHop hop[IntHeader::maxHop];
    uint32_t keep[IntHeader::maxHop];
    uint32_t m_incStage;
    double m_lastGap;
    double u;
    struct {
        double u;
        DataRate Rc;
        uint32_t incStage;
    } hopState[IntHeader::maxHop];
};

using HPCCPint = struct hpcc_pint {
    uint32_t m_lastUpdateSeq;
    DataRate m_curRate;
    uint32_t m_incStage;
};

using TIMELY = struct timely {
    uint32_t m_lastUpdateSeq;
    DataRate m_curRate;
    uint32_t m_incStage;
    uint64_t lastRtt;
    double rttDiff;
};

using DCTCP = struct dctcp {
    uint32_t m_lastUpdateSeq;
    uint32_t m_caState;
    uint32_t m_highSeq;  // when to exit cwr
    double m_alpha;
    uint32_t m_ecnCnt;
    uint32_t m_batchSizeOfAlpha;
};

using ECNAccount = struct ecn_account {
    uint16_t qIndex;
    uint8_t ecnbits;
    uint16_t qfb;
    uint16_t total;

    ecn_account() { memset(this, 0, sizeof(ecn_account)); }
};

}  // namespace CongestionControl

class CongestionControlSender {
   public:
    bool IsWinBound();
    uint64_t GetWin();  // window size calculated from m_rate
    virtual uint64_t GetOnTheFly();
    uint64_t HpGetCurWin();  // window size calculated from hp.m_curRate, used by HPCC

    void SetWin(uint32_t win);
    void SetBaseRtt(uint64_t baseRtt);
    void SetVarWin(bool v);

    CongestionControl::DCQCN mlx;
    CongestionControl::HPCC hp;
    CongestionControl::TIMELY tmly;
    CongestionControl::DCTCP dctcp;
    CongestionControl::HPCCPint hpccPint;

    uint64_t m_baseRtt;   // base RTT of this qp
    DataRate m_max_rate;  // max rate
    Time m_nextAvail;     //< Soonest time of next send
    DataRate m_rate;      //< Current rate

    // flow control
    uint32_t m_win;  // bound of on-the-fly packets
    uint32_t wp;     // current window of packets
    bool m_var_win;  // variable window size
};

class CongestionControlReceiver {
   public:
    CongestionControl::ECNAccount m_ecn_source;
    EventId QcnTimerEvent;  // if destroy this rxQp, remember to cancel this timer
};

class RdmaQueuePair : public Object, public CongestionControlSender {
   public:
    // app-specified
    Time startTime;
    Callback<void> m_notifyAppFinish;
    Callback<void, IBVWorkCompletion&> m_notifyCompletion;

    // connection
    QPConnectionAttr m_connectionAttr;
    uint16_t m_ipid;

    uint64_t m_size;

    // reliability
    uint64_t snd_nxt, snd_una;  // next seq to send, the highest unacked seq
    uint32_t lastPktSize;

    /***********
     * methods
     **********/
    static TypeId GetTypeId(void);

    RdmaQueuePair(const QPConnectionAttr& attr);

    void SetSize(uint64_t size);
    void SetAppNotifyCallback(Callback<void> notifyAppFinish);
    void SetCompletionCallback(Callback<void, IBVWorkCompletion&> notifyAPPCompletion);
    uint64_t GetBytesLeft();
    bool IsFinished();
    virtual uint64_t GetOnTheFly() override final;
    uint32_t GetHash(void);
    void Acknowledge(uint64_t ack);
};

class RdmaRxQueuePair : public Object, public CongestionControlReceiver {  // Rx side queue pair
   public:
    // connection
    QPConnectionAttr m_connectionAttr;

    uint16_t m_ipid;

    // reliability
    Time m_nackTimer;
    uint32_t ReceiverNextExpectedSeq;
    int32_t m_milestone_rx;
    uint32_t m_lastNACK;

    static TypeId GetTypeId(void);
    RdmaRxQueuePair();
    uint32_t GetHash(void);
};

class RdmaQueuePairGroup : public Object {
   public:
    std::vector<Ptr<RdmaQueuePair> > m_qps;
    // std::vector<Ptr<RdmaRxQueuePair> > m_rxQps;

    static TypeId GetTypeId(void);
    RdmaQueuePairGroup(void);
    uint32_t GetN(void);
    Ptr<RdmaQueuePair> Get(uint32_t idx);
    Ptr<RdmaQueuePair> operator[](uint32_t idx);
    void AddQp(Ptr<RdmaQueuePair> qp);
    // void AddRxQp(Ptr<RdmaRxQueuePair> rxQp);
    void Clear(void);
};

}  // namespace ns3
#endif /* RDMA_QUEUE_PAIR_H */
