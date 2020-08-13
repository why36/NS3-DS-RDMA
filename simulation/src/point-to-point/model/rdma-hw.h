#ifndef RDMA_HW_H
#define RDMA_HW_H

#include <ns3/custom-header.h>
#include <ns3/ipv4-flow-classifier.h>
#include <ns3/node.h>
#include <ns3/rdma-queue-pair.h>
#include <ns3/rdma.h>

#include <unordered_map>

#include "pint.h"
#include "qbb-net-device.h"

namespace ns3
{

    struct RdmaInterfaceMgr
    {
        Ptr<QbbNetDevice> dev;
        Ptr<QueuePairSet> qpGrp;

        RdmaInterfaceMgr() : dev(NULL), qpGrp(NULL) {}
        RdmaInterfaceMgr(Ptr<QbbNetDevice> _dev) {
            dev = _dev;
        }
    };

    enum class RCSeqState
    {
        ERROR = 0,
        GENERATE_ACK,
        GENERATE_NACK,
        DUPLICATED,
        NOT_GENERATE_NACK,
        NOT_GENERATE_ACK
    };
    enum class UCSeqState
    {
        ERROR = 0,
        OK,
        OOS,
        DUPLICATED
    };

    class RdmaHw : public Object
    {
    public:
        static TypeId GetTypeId(void);
        RdmaHw();

        Ptr<Node> m_node;
        DataRate m_minRate; //< Min sending rate
        uint32_t m_mtu;
        uint32_t m_cc_mode;
        uint8_t m_CCType; // can not used CongestionControlType since attributed should be tracked
        double m_nack_interval;
        uint32_t m_chunk;
        uint32_t m_ack_interval;
        bool m_backto0;
        bool m_var_win, m_fast_react;
        bool m_rateBound;
        std::vector<RdmaInterfaceMgr> m_nic;                                                            // list of running nic controlled by this RdmaHw
        std::unordered_map<SimpleTuple, Ptr<RdmaQueuePair>, SimpleTupleHash, SimpleTupleEqual> m_qpMap; // mapping from uint64_t to qp
        std::unordered_map<uint32_t, std::vector<int>> m_rtTable;                                       // map from ip address (u32) to possible ECMP port (index of dev)

        std::unordered_map<SimpleTuple, Ptr<CongestionControlEntity>, SimpleTupleHash, SimpleTupleEqual> m_CCRateMap;

        // qp complete callback
        typedef Callback<void, Ptr<RdmaQueuePair>> QpCompleteCallback;
        QpCompleteCallback m_qpCompleteCallback;

        void SetNode(Ptr<Node> node);
        void Setup(QpCompleteCallback cb); // setup shared data and callbacks with the QbbNetDevice
        // static uint64_t GetQpKey(uint32_t dip, uint16_t sport, uint16_t pg);  // get the lookup key for m_qpMap
        Ptr<RdmaQueuePair> GetQp(SimpleTuple &tuple);  // get the qp
        uint32_t GetNicIdxOfQp(Ptr<RdmaQueuePair> qp); // get the NIC index of the qp

        Ptr<RdmaQueuePair> AddQueuePair(uint64_t size, const QPConnectionAttr &conn_attr, uint32_t win, uint64_t baseRtt, Callback<void> notifyAppFinish,
            Callback<void, Ptr<IBVWorkCompletion>> notifyCompletion); // add a new qp (new send)
        void DeleteQueuePair(Ptr<RdmaQueuePair> qp);
        void DeleteQp(uint32_t sip, uint32_t dip, uint16_t sport, uint16_t dport, uint16_t pg);

        // TO DO Krayecho Yx: these fuunctions should be moved into QPs
        int Receive(Ptr<Packet> p, CustomHeader &ch); // callback function that the QbbNetDevice should use when receive packets. Only NIC can call this
                                                      // function. And do not call this upon PFC
        //void RCReceiveUdp(Ptr<Packet> p, Ptr<RdmaQueuePair> qp, CustomHeader &ch);
        //void UCReceiveUdp(Ptr<Packet> p, Ptr<RdmaQueuePair> qp, CustomHeader &ch);
        bool CheckOpcodeValid(IBHeader ibh, OpCodeType type);
        bool CheckOpcodeOperationSupported(IBHeader ibh);
        UCSeqState MatchFirstOrOnly(IBHeader ibh);
        bool UCCheckOpcodeSequence(IBHeader ibh, Ptr<RdmaQueuePair> rxQp);

        void RCReceiveInboundRequest(Ptr<Packet> p, Ptr<RdmaQueuePair> qp, CustomHeader &ch);
        void UCReceiveInboundRequest(Ptr<Packet> p, Ptr<RdmaQueuePair> qp, CustomHeader &ch);

        int ReceiveCnp(Ptr<Packet> p, CustomHeader &ch);
        int ReceiveAck(Ptr<Packet> p, CustomHeader &ch); // handle both ACK and NACK

        void CheckandSendQCN(Ptr<RdmaQueuePair> q);

        RCSeqState RCReceiverCheckSeq(uint32_t seq, Ptr<RdmaQueuePair> q, uint32_t size);
        UCSeqState UCReceiverCheckSeq(CustomHeader &header, Ptr<RdmaQueuePair> q, uint32_t size);

        void AddHeader(Ptr<Packet> p, uint16_t protocolNumber);
        static uint16_t EtherToPpp(uint16_t protocol);

        void RecoverQueue(Ptr<RdmaQueuePair> qp);
        void QpComplete(Ptr<RdmaQueuePair> qp);
        void SetLinkDown(Ptr<QbbNetDevice> dev);

        // call this function after the NIC is setup
        void AddTableEntry(Ipv4Address &dstAddr, uint32_t intf_idx);
        void ClearTable();
        void RedistributeQp();

        Ptr<Packet> GetNxtPacket(Ptr<RdmaQueuePair> qp); // get next packet to send, inc snd_nxt
        void PktSent(Ptr<RdmaQueuePair> qp, Ptr<Packet> pkt, Time interframeGap);
        void UpdateNextAvail(Ptr<RdmaQueuePair> qp, Time interframeGap, uint32_t pkt_size);
        void ChangeRate(Ptr<CongestionControlEntity> qp, DataRate new_rate);
        /******************************
     * Mellanox's version of DCQCN
     *****************************/
        double m_g;              // feedback weight
        double m_rateOnFirstCNP; // the fraction of line rate to set on first CNP
        bool m_EcnClampTgtRate;
        double m_rpgTimeReset;
        double m_rateDecreaseInterval;
        uint32_t m_rpgThreshold;
        double m_alpha_resume_interval;
        DataRate m_rai;  //< Rate of additive increase
        DataRate m_rhai; //< Rate of hyper-additive increase

        // the Mellanox's version of alpha update:
        // every fixed time slot, update alpha.
        void UpdateAlphaMlx(Ptr<CongestionControlEntity> q);
        void ScheduleUpdateAlphaMlx(Ptr<CongestionControlEntity> q);

        // Mellanox's version of CNP receive
        void cnp_received_mlx(Ptr<CongestionControlEntity> q);

        // Mellanox's version of rate decrease
        // It checks every m_rateDecreaseInterval if CNP arrived (m_decrease_cnp_arrived).
        // If so, decrease rate, and reset all rate increase related things
        void CheckRateDecreaseMlx(Ptr<CongestionControlEntity> q);
        void ScheduleDecreaseRateMlx(Ptr<CongestionControlEntity> q, uint32_t delta);

        // Mellanox's version of rate increase
        void RateIncEventTimerMlx(Ptr<CongestionControlEntity> q);
        void RateIncEventMlx(Ptr<CongestionControlEntity> q);
        void FastRecoveryMlx(Ptr<CongestionControlEntity> q);
        void ActiveIncreaseMlx(Ptr<CongestionControlEntity> q);
        void HyperIncreaseMlx(Ptr<CongestionControlEntity> q);

        /***********************
     * High Precision CC
     ***********************/
        double m_targetUtil;
        double m_utilHigh;
        uint32_t m_miThresh;
        bool m_multipleRate;
        bool m_sampleFeedback; // only react to feedback every RTT, or qlen > 0
        void HandleAckHp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);
        void UpdateRateHp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool fast_react);
        void UpdateRateHpTest(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool fast_react);
        void FastReactHp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);

        /**********************
     * TIMELY
     *********************/
        double m_tmly_alpha, m_tmly_beta;
        uint64_t m_tmly_TLow, m_tmly_THigh, m_tmly_minRtt;
        void HandleAckTimely(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);
        void UpdateRateTimely(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool us);
        void FastReactTimely(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);

        /**********************
     * DCTCP
     *********************/
        DataRate m_dctcp_rai;
        void HandleAckDctcp(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);

        /*********************
     * HPCC-PINT
     ********************/
        uint32_t pint_smpl_thresh;
        void SetPintSmplThresh(double p);
        void HandleAckHpPint(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch);
        void UpdateRateHpPint(Ptr<RdmaQueuePair> qp, Ptr<Packet> p, CustomHeader &ch, bool fast_react);
    };

} /* namespace ns3 */

#endif /* RDMA_HW_H */
