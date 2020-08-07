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
#include <ns3/tag.h>

#include <queue>
#include <vector>

namespace ns3
{

    enum class CongestionControlType
    {
        IPBase = 0,
        FlowBase = 1
    };

    namespace CongestionControlAlgorithms
    {
        using DCQCN = struct dcqcn
        {
            DataRate m_targetRate; //< Target rate
            EventId m_eventUpdateAlpha;
            double m_alpha;
            bool m_alpha_cnp_arrived; // indicate if CNP arrived in the last slot
            bool m_first_cnp;         // indicate if the current CNP is the first CNP
            EventId m_eventDecreaseRate;
            bool m_decrease_cnp_arrived; // indicate if CNP arrived in the last slot
            uint32_t m_rpTimeStage;
            EventId m_rpTimer;
        };

        using HPCC = struct hpcc
        {
            uint32_t m_lastUpdateSeq;
            DataRate m_curRate;
            IntHop hop[IntHeader::maxHop];
            uint32_t keep[IntHeader::maxHop];
            uint32_t m_incStage;
            double m_lastGap;
            double u;
            struct
            {
                double u;
                DataRate Rc;
                uint32_t incStage;
            } hopState[IntHeader::maxHop];
        };

        using HPCCPint = struct hpcc_pint
        {
            uint32_t m_lastUpdateSeq;
            DataRate m_curRate;
            uint32_t m_incStage;
        };

        using TIMELY = struct timely
        {
            uint32_t m_lastUpdateSeq;
            DataRate m_curRate;
            uint32_t m_incStage;
            uint64_t lastRtt;
            double rttDiff;
        };

        using DCTCP = struct dctcp
        {
            uint32_t m_lastUpdateSeq;
            uint32_t m_caState;
            uint32_t m_highSeq; // when to exit cwr
            double m_alpha;
            uint32_t m_ecnCnt;
            uint32_t m_batchSizeOfAlpha;
        };

        using ECNAccount = struct ecn_account
        {
            uint16_t qIndex;
            uint8_t ecnbits;
            uint16_t qfb;
            uint16_t total;

            ecn_account() {
                memset(this, 0, sizeof(ecn_account));
            }
        };

    } // namespace CongestionControlAlgorithms

    class RdmaHw;
    class RdmaQueuePair;

    using IPBasedFlow = struct ip_based_flow
    {
        uint16_t pg;
        Ipv4Address sip;
        Ipv4Address dip;
    };

    using IPBasedFlowHash = struct ip_based_flow_hash
    {
        std::size_t operator()(const IPBasedFlow &s) const
        {
            uint32_t magic = (2 << 12) - 1;
            return ((s.sip.Get() | magic) << 24) + ((s.dip.Get() | magic) << 12) + static_cast<uint32_t>(static_cast<uint8_t>(s.pg));
        }
    };

    using IPBasedFlowEqual = struct ip_based_flow_equal
    {
        std::size_t operator()(const IPBasedFlow &lhs, const IPBasedFlow &rhs) const
        {
            return ((lhs.sip == rhs.sip) && (lhs.dip == rhs.dip) && (lhs.pg == rhs.pg));
        }
    };

    class CongestionControlEntity : public virtual Object
    {
    public:
        CongestionControlEntity();
        bool IsWinBound();
        uint64_t GetWin(); // window size calculated from m_rate
        virtual uint64_t GetOnTheFly();
        virtual uint64_t GetBytesLeft();
        virtual bool IsFinished();
        uint64_t HpGetCurWin(); // window size calculated from hp.m_curRate, used by HPCC
        static TypeId GetTypeId(void);

        void SetWin(uint32_t win);
        void SetBaseRtt(uint64_t baseRtt);
        void SetVarWin(bool v);

        CongestionControlAlgorithms::DCQCN mlx;
        CongestionControlAlgorithms::HPCC hp;
        CongestionControlAlgorithms::TIMELY tmly;
        CongestionControlAlgorithms::DCTCP dctcp;
        CongestionControlAlgorithms::HPCCPint hpccPint;

        uint64_t m_baseRtt; // base RTT of this qp
        // flow control
        uint32_t m_win; // bound of on-the-fly packets
        uint32_t wp;    // current window of packets
        bool m_var_win; // variable window size

        DataRate m_max_rate;  // max rate
        DataRate m_rate;      //< Current rate
        uint32_t lastPktSize; // last packet size

        CongestionControlAlgorithms::ECNAccount m_ecn_source;
        EventId QcnTimerEvent; // if destroy this rxQp, remember to cancel this timer
    };

    class QueuePairSet : public Object
    {
    public:
        CongestionControlType mCCType;
        std::vector<Ptr<RdmaQueuePair>> m_qps;
        static TypeId GetTypeId(void);
        QueuePairSet(void);
        uint32_t GetN(void);
        Ptr<RdmaQueuePair> Get(uint32_t idx);
        Ptr<RdmaQueuePair> operator[](uint32_t idx);
        void AddQp(Ptr<RdmaQueuePair> qp);
        void Clear(void);
    };

    class RdmaQueuePair : public Object
    {
    public:
        // app-specified
        Time startTime;
        Callback<void> m_notifyAppFinish;
        Callback<void, Ptr<IBVWorkCompletion>> m_notifyCompletion;

        // connection
        QPConnectionAttr m_connectionAttr;
        uint16_t m_ipid;

        uint64_t m_size;

        // reliability sender
        uint64_t snd_nxt, snd_una; // next seq to send, the highest unacked seq

        // reliability receiver
        Time m_nackTimer;
        uint32_t ReceiverNextExpectedSeq;
        int32_t m_milestone_rx;
        uint32_t m_lastNACK;

        // congestion control sender
        Ptr<CongestionControlEntity> m_CCEntity;

        Time m_nextAvail; //< Soonest time of next send

        // data path
        std::queue<Ptr<IBVWorkRequest>> m_wrs;
        std::queue<Ptr<IBVWorkRequest>> m_receive_wrs;
        Ptr<IBVWorkRequest> m_sendingWr;
        Ptr<IBVWorkRequest> m_receiveWr;
        uint32_t m_remainingSize;
        OpCodeOperation m_sendingOperation;

        RdmaHw *m_rdma;
        // uint32_t m_mtu;
        /***********
     * methods
     **********/
        static TypeId GetTypeId(void);

        RdmaQueuePair(const QPConnectionAttr &attr);

        void SetSize(uint64_t size);
        void SetAppNotifyCallback(Callback<void> notifyAppFinish);
        void SetCompletionCallback(Callback<void, Ptr<IBVWorkCompletion>> notifyAPPCompletion);
        virtual uint64_t GetBytesLeft();
        virtual bool IsFinished();
        virtual uint64_t GetOnTheFly();
        uint32_t GetHash(void);
        void Acknowledge(uint64_t ack);

        // data path
        int ibv_post_send(Ptr<IBVWorkRequest> wc);
        Ptr<Packet> GetNextPacket();
        bool GetNextIbvRequest_AssemblePacket_Finished(Ptr<Packet> p, Ptr<IBVWorkRequest> &m_receiveWr);
        int Empty();
    };

    /*
class IPBasedCongestionControlEntity : public CongestionControlEntity {
public:
    virtual Ptr<RdmaQueuePair> GetNextQp() override;
    static TypeId GetTypeId(void);
    void SetIPBasedFlow(QPConnectionAttr& attr);
    IPBasedFlow mIPBasedFlow;
    std::vector<Ptr<RdmaQueuePair>> m_QPs;

   private:
    uint32_t m_lastQP = 0;
};
*/
} // namespace ns3
#endif /* RDMA_QUEUE_PAIR_H */
