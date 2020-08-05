#include "rdma-hw.h"

#include <ns3/ipv4-header.h>
#include <ns3/rdma-queue-pair.h>
#include <ns3/seq-ts-header.h>
#include <ns3/simulator.h>
#include <ns3/udp-header.h>

#include "cn-header.h"
#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/ppp-header.h"
#include "ns3/uinteger.h"
#include "ppp-header.h"
#include "qbb-header.h"

namespace ns3
{

    TypeId RdmaHw::GetTypeId(void)
    {
        static TypeId tid =
            TypeId("ns3::RdmaHw")
                .SetParent<Object>()
                .AddAttribute("MinRate", "Minimum rate of a throttled flow", DataRateValue(DataRate("100Mb/s")), MakeDataRateAccessor(&RdmaHw::m_minRate),
                              MakeDataRateChecker())
                .AddAttribute("Mtu", "Mtu.", UintegerValue(1000), MakeUintegerAccessor(&RdmaHw::m_mtu), MakeUintegerChecker<uint32_t>())
                .AddAttribute("CcMode", "which mode of DCQCN is running", UintegerValue(0), MakeUintegerAccessor(&RdmaHw::m_cc_mode),
                              MakeUintegerChecker<uint32_t>())
                .AddAttribute("CCType", "IPbased or Flowbased congestion control", UintegerValue(static_cast<uint8_t>(CongestionControlType::FlowBase)), MakeUintegerAccessor(&RdmaHw::m_CCType),
                              MakeUintegerChecker<uint8_t>())
                .AddAttribute("NACK Generation Interval", "The NACK Generation interval", DoubleValue(500.0),
                              MakeDoubleAccessor(&RdmaHw::m_nack_interval), MakeDoubleChecker<double>())
                .AddAttribute("L2ChunkSize", "Layer 2 chunk size. Disable chunk mode if equals to 0.", UintegerValue(0),
                              MakeUintegerAccessor(&RdmaHw::m_chunk), MakeUintegerChecker<uint32_t>())
                .AddAttribute("L2AckInterval", "Layer 2 Ack intervals. Disable ack if equals to 0.", UintegerValue(0),
                              MakeUintegerAccessor(&RdmaHw::m_ack_interval), MakeUintegerChecker<uint32_t>())
                .AddAttribute("L2BackToZero", "Layer 2 go back to zero transmission.", BooleanValue(false), MakeBooleanAccessor(&RdmaHw::m_backto0),
                              MakeBooleanChecker())
                .AddAttribute("EwmaGain", "Control gain parameter which determines the level of rate decrease", DoubleValue(1.0 / 16),
                              MakeDoubleAccessor(&RdmaHw::m_g), MakeDoubleChecker<double>())
                .AddAttribute("RateOnFirstCnp", "the fraction of rate on first CNP", DoubleValue(1.0), MakeDoubleAccessor(&RdmaHw::m_rateOnFirstCNP),
                              MakeDoubleChecker<double>())
                .AddAttribute("ClampTargetRate", "Clamp target rate.", BooleanValue(false), MakeBooleanAccessor(&RdmaHw::m_EcnClampTgtRate),
                              MakeBooleanChecker())
                .AddAttribute("RPTimer", "The rate increase timer at RP in microseconds", DoubleValue(1500.0),
                              MakeDoubleAccessor(&RdmaHw::m_rpgTimeReset), MakeDoubleChecker<double>())
                .AddAttribute("RateDecreaseInterval", "The interval of rate decrease check", DoubleValue(4.0),
                              MakeDoubleAccessor(&RdmaHw::m_rateDecreaseInterval), MakeDoubleChecker<double>())
                .AddAttribute("FastRecoveryTimes", "The rate increase timer at RP", UintegerValue(5), MakeUintegerAccessor(&RdmaHw::m_rpgThreshold),
                              MakeUintegerChecker<uint32_t>())
                .AddAttribute("AlphaResumInterval", "The interval of resuming alpha", DoubleValue(55.0),
                              MakeDoubleAccessor(&RdmaHw::m_alpha_resume_interval), MakeDoubleChecker<double>())
                .AddAttribute("RateAI", "Rate increment unit in AI period", DataRateValue(DataRate("5Mb/s")), MakeDataRateAccessor(&RdmaHw::m_rai),
                              MakeDataRateChecker())
                .AddAttribute("RateHAI", "Rate increment unit in hyperactive AI period", DataRateValue(DataRate("50Mb/s")),
                              MakeDataRateAccessor(&RdmaHw::m_rhai), MakeDataRateChecker())
                .AddAttribute("VarWin", "Use variable window size or not", BooleanValue(false), MakeBooleanAccessor(&RdmaHw::m_var_win),
                              MakeBooleanChecker())
                .AddAttribute("FastReact", "Fast React to congestion feedback", BooleanValue(true), MakeBooleanAccessor(&RdmaHw::m_fast_react),
                              MakeBooleanChecker())
                .AddAttribute("MiThresh", "Threshold of number of consecutive AI before MI", UintegerValue(5), MakeUintegerAccessor(&RdmaHw::m_miThresh),
                              MakeUintegerChecker<uint32_t>())
                .AddAttribute("TargetUtil", "The Target Utilization of the bottleneck bandwidth, by default 95%", DoubleValue(0.95),
                              MakeDoubleAccessor(&RdmaHw::m_targetUtil), MakeDoubleChecker<double>())
                .AddAttribute("UtilHigh", "The upper bound of Target Utilization of the bottleneck bandwidth, by default 98%", DoubleValue(0.98),
                              MakeDoubleAccessor(&RdmaHw::m_utilHigh), MakeDoubleChecker<double>())
                .AddAttribute("RateBound", "Bound packet sending by rate, for test only", BooleanValue(true), MakeBooleanAccessor(&RdmaHw::m_rateBound),
                              MakeBooleanChecker())
                .AddAttribute("MultiRate", "Maintain multiple rates in HPCC", BooleanValue(true), MakeBooleanAccessor(&RdmaHw::m_multipleRate),
                              MakeBooleanChecker())
                .AddAttribute("SampleFeedback", "Whether sample feedback or not", BooleanValue(false), MakeBooleanAccessor(&RdmaHw::m_sampleFeedback),
                              MakeBooleanChecker())
                .AddAttribute("TimelyAlpha", "Alpha of TIMELY", DoubleValue(0.875), MakeDoubleAccessor(&RdmaHw::m_tmly_alpha),
                              MakeDoubleChecker<double>())
                .AddAttribute("TimelyBeta", "Beta of TIMELY", DoubleValue(0.8), MakeDoubleAccessor(&RdmaHw::m_tmly_beta), MakeDoubleChecker<double>())
                .AddAttribute("TimelyTLow", "TLow of TIMELY (ns)", UintegerValue(50000), MakeUintegerAccessor(&RdmaHw::m_tmly_TLow),
                              MakeUintegerChecker<uint64_t>())
                .AddAttribute("TimelyTHigh", "THigh of TIMELY (ns)", UintegerValue(500000), MakeUintegerAccessor(&RdmaHw::m_tmly_THigh),
                              MakeUintegerChecker<uint64_t>())
                .AddAttribute("TimelyMinRtt", "MinRtt of TIMELY (ns)", UintegerValue(20000), MakeUintegerAccessor(&RdmaHw::m_tmly_minRtt),
                              MakeUintegerChecker<uint64_t>())
                .AddAttribute("DctcpRateAI", "DCTCP's Rate increment unit in AI period", DataRateValue(DataRate("1000Mb/s")),
                              MakeDataRateAccessor(&RdmaHw::m_dctcp_rai), MakeDataRateChecker())
                .AddAttribute("PintSmplThresh", "PINT's sampling threshold in rand()%65536", UintegerValue(65536),
                              MakeUintegerAccessor(&RdmaHw::pint_smpl_thresh), MakeUintegerChecker<uint32_t>());
        return tid;
    }

    RdmaHw::RdmaHw() {}

    void RdmaHw::SetNode(Ptr<Node> node) { m_node = node; }
    void RdmaHw::Setup(QpCompleteCallback cb)
    {
        for (uint32_t i = 0; i < m_nic.size(); i++)
        {
            Ptr<QbbNetDevice> dev = m_nic[i].dev;
            if (dev == NULL)
                continue;
            // share data with NIC
            dev->m_rdmaEQ->m_qpGrp = m_nic[i].qpGrp;
            // setup callback
            dev->m_rdmaReceiveCb = MakeCallback(&RdmaHw::Receive, this);
            dev->m_rdmaLinkDownCb = MakeCallback(&RdmaHw::SetLinkDown, this);
            dev->m_rdmaPktSent = MakeCallback(&RdmaHw::PktSent, this);
            // config NIC
            dev->m_rdmaEQ->m_rdmaGetNxtPkt = MakeCallback(&RdmaHw::GetNxtPacket, this);
        }
        // setup qp complete callback
        m_qpCompleteCallback = cb;
    }

    uint32_t RdmaHw::GetNicIdxOfQp(Ptr<RdmaQueuePair> qp)
    {
        auto &v = m_rtTable[qp->m_connectionAttr.dip.Get()];
        if (v.size() > 0)
        {
            return v[qp->GetHash() % v.size()];
        }
        else
        {
            NS_ASSERT_MSG(false, "We assume at least one NIC is alive");
        }
    }
    // uint64_t RdmaHw::GetQpKey(uint32_t dip, uint16_t sport, uint16_t pg) { return ((uint64_t)dip << 32) | ((uint64_t)sport << 16) | (uint64_t)pg; }

    Ptr<RdmaQueuePair> RdmaHw::GetQp(SimpleTuple &tuple)
    {
        auto it = m_qpMap.find(tuple);
        if (it != m_qpMap.end())
            return it->second;
        return NULL;
    }
    Ptr<RdmaQueuePair> RdmaHw::AddQueuePair(uint64_t size, const QPConnectionAttr &attr, uint32_t win, uint64_t baseRtt, Callback<void> notifyAppFinish,
                                            Callback<void, Ptr<IBVWorkCompletion>> notifyCompletion)
    {
        // create qp

        Ptr<RdmaQueuePair> qp = CreateObject<RdmaQueuePair>(attr);
        SimpleTuple tuple = {
            .sip = attr.sip.Get(),
            .dip = attr.dip.Get(),
            .sport = attr.sport,
            .dport = attr.dport,
            .prio = attr.pg,
        };
        m_qpMap[tuple] = qp;
        uint32_t nic_idx = GetNicIdxOfQp(qp);
        m_nic[nic_idx].qpGrp->AddQp(qp);

        if (static_cast<CongestionControlType>(m_CCType) == CongestionControlType::IPBase)
        {
            tuple.sport = 0;
            tuple.dport = 0;
        }

        qp->SetSize(size);

        if (m_CCRateMap.count(tuple) == 0)
        {
            m_CCRateMap[tuple] = Create<CongestionControlEntity>();
            qp->m_CCEntity->SetWin(win);
            qp->m_CCEntity->SetBaseRtt(baseRtt);
            qp->m_CCEntity->SetVarWin(m_var_win);
            qp->SetAppNotifyCallback(notifyAppFinish);
        }

        Ptr<CongestionControlEntity> cc_entity = m_CCRateMap[tuple];
        qp->m_CCEntity = cc_entity;

        // set init variables
        DataRate m_bps = m_nic[nic_idx].dev->GetDataRate();
        cc_entity->m_rate = m_bps;
        cc_entity->m_max_rate = m_bps;
        if (m_cc_mode == 1)
        {
            cc_entity->mlx.m_targetRate = m_bps;
        }
        else if (m_cc_mode == 3)
        {
            cc_entity->hp.m_curRate = m_bps;
            if (m_multipleRate)
            {
                for (uint32_t i = 0; i < IntHeader::maxHop; i++)
                    cc_entity->hp.hopState[i].Rc = m_bps;
            }
        }
        else if (m_cc_mode == 7)
        {
            cc_entity->tmly.m_curRate = m_bps;
        }
        else if (m_cc_mode == 10)
        {
            cc_entity->hpccPint.m_curRate = m_bps;
        }

        // Notify Nic
        m_nic[nic_idx].dev->NewQp(qp);
        return qp;
    }

    void RdmaHw::DeleteQueuePair(Ptr<RdmaQueuePair> qp)
    {
        // remove qp from the m_qpMap
        QPConnectionAttr &attr = qp->m_connectionAttr;
        SimpleTuple tuple = {
            .sip = attr.sip.Get(),
            .dip = attr.dip.Get(),
            .sport = attr.sport,
            .dport = attr.dport,
            .prio = attr.pg,
        };
        m_qpMap.erase(tuple);
    }

    void RdmaHw::DeleteQp(uint32_t sip, uint32_t dip, uint16_t sport, uint16_t dport, uint16_t pg)
    {
        SimpleTuple tuple = {.sip = sip, .dip = dip, .sport = sport, .dport = dport, .prio = pg};

        NS_ASSERT_MSG(m_qpMap.count(tuple), "Cannot find rxQP when deleting");
        m_qpMap.erase(tuple);
    }

    void RdmaHw::RCReceiveUdp(Ptr<Packet> p, Ptr<RdmaQueuePair> rxQp, CustomHeader &ch)
    {
        uint8_t ecnbits = ch.GetIpv4EcnBits();
        uint32_t payload_size = p->GetSize() - ch.GetSerializedSize();

        if (ecnbits != 0)
        {
            rxQp->m_CCEntity->m_ecn_source.ecnbits |= ecnbits;
            rxQp->m_CCEntity->m_ecn_source.qfb++;
        }
        rxQp->m_CCEntity->m_ecn_source.total++;
        rxQp->m_milestone_rx = m_ack_interval;

        RCSeqState x = RCReceiverCheckSeq(ch.udp.seq, rxQp, payload_size);
        switch (x)
        {
        case RCSeqState::GENERATE_ACK:
        case RCSeqState::GENERATE_NACK:
        {
            qbbHeader seqh;
            seqh.SetSeq(rxQp->ReceiverNextExpectedSeq);
            seqh.SetPG(ch.udp.pg);
            seqh.SetSport(ch.udp.dport);
            seqh.SetDport(ch.udp.sport);
            seqh.SetIntHeader(ch.udp.ih);
            if (ecnbits)
                seqh.SetCnp();

            Ptr<Packet> newp = Create<Packet>(std::max(60 - 14 - 20 - (int)seqh.GetSerializedSize(), 0));
            newp->AddHeader(seqh);

            Ipv4Header head; // Prepare IPv4 header
            head.SetDestination(Ipv4Address(ch.sip));
            head.SetSource(Ipv4Address(ch.dip));
            head.SetProtocol(x == RCSeqState::GENERATE_ACK ? 0xFC : 0xFD); // ack=0xFC nack=0xFD
            head.SetTtl(64);
            head.SetPayloadSize(newp->GetSize());
            head.SetIdentification(rxQp->m_ipid++);

            newp->AddHeader(head);
            AddHeader(newp, 0x800); // Attach PPP header
            // send
            uint32_t nic_idx = GetNicIdxOfQp(rxQp);
            m_nic[nic_idx].dev->RdmaEnqueueHighPrioQ(newp);
            m_nic[nic_idx].dev->TriggerTransmit();
        }
        }
    }

    void RdmaHw::UCReceiveUdp(Ptr<Packet> p, Ptr<RdmaQueuePair> rxQp, CustomHeader &ch)
    {
        uint8_t ecnbits = ch.GetIpv4EcnBits();

        uint32_t payload_size = p->GetSize() - ch.GetSerializedSize();

        // TODO find corresponding rx queue pair
        if (ecnbits != 0)
        {
            rxQp->m_CCEntity->m_ecn_source.ecnbits |= ecnbits;
            rxQp->m_CCEntity->m_ecn_source.qfb++;
        }
        rxQp->m_CCEntity->m_ecn_source.total++;
        rxQp->m_milestone_rx = m_ack_interval;

        // TO DO Krayecho Yx: implement UC receiver logic
        UCSeqState x = UCReceiverCheckSeq(ch, rxQp, payload_size);
        return;
    }

    int RdmaHw::ReceiveCnp(Ptr<Packet> p, CustomHeader &ch)
    {
        // QCN on NIC
        // This is a Congestion signal
        // Then, extract data from the congestion packet.
        // We assume, without verify, the packet is destinated to me
        uint32_t qIndex = ch.cnp.qIndex;
        if (qIndex == 1)
        { // DCTCP
            std::cout << "TCP--ignore\n";
            return 0;
        }
        uint16_t udpport = ch.cnp.fid; // corresponds to the sport
        uint8_t ecnbits = ch.cnp.ecnBits;
        uint16_t qfb = ch.cnp.qfb;
        uint16_t total = ch.cnp.total;

        uint32_t i;
        // get qp
        SimpleTuple rxTuple = {
            .sip = ch.dip,
            .dip = ch.sip,
            .sport = ch.udp.dport,
            .dport = ch.udp.sport,
            .prio = ch.udp.pg,
        };
        Ptr<RdmaQueuePair> qp = GetQp(rxTuple);
        if (qp == NULL)
            std::cout << "ERROR: QCN NIC cannot find the flow\n";
        // get nic
        uint32_t nic_idx = GetNicIdxOfQp(qp);
        Ptr<QbbNetDevice> dev = m_nic[nic_idx].dev;

        if (qp->m_CCEntity->m_rate == 0) // lazy initialization
        {
            qp->m_CCEntity->m_rate = dev->GetDataRate();
            if (m_cc_mode == 1)
            {
                qp->m_CCEntity->mlx.m_targetRate = dev->GetDataRate();
            }
            else if (m_cc_mode == 3)
            {
                qp->m_CCEntity->hp.m_curRate = dev->GetDataRate();
                if (m_multipleRate)
                {
                    for (uint32_t i = 0; i < IntHeader::maxHop; i++)
                        qp->m_CCEntity->hp.hopState[i].Rc = dev->GetDataRate();
                }
            }
            else if (m_cc_mode == 7)
            {
                qp->m_CCEntity->tmly.m_curRate = dev->GetDataRate();
            }
            else if (m_cc_mode == 10)
            {
                qp->m_CCEntity->hpccPint.m_curRate = dev->GetDataRate();
            }
        }
        return 0;
    } // namespace ns3

    int RdmaHw::ReceiveAck(Ptr<Packet> p, CustomHeader &ch)
    {
        uint16_t qIndex = ch.ack.pg;
        uint16_t port = ch.ack.dport;
        uint32_t seq = ch.ack.seq;
        uint8_t cnp = (ch.ack.flags >> qbbHeader::FLAG_CNP) & 1;
        int i;
        SimpleTuple rxTuple = {
            .sip = ch.sip,
            .dip = ch.dip,
            .sport = ch.udp.sport,
            .dport = ch.udp.dport,
            .prio = ch.udp.pg,
        };
        Ptr<RdmaQueuePair> qp = GetQp(rxTuple);
        if (qp == NULL)
        {
            std::cout << "ERROR: "
                      << "node:" << m_node->GetId() << ' ' << (ch.l3Prot == 0xFC ? "ACK" : "NACK") << " NIC cannot find the flow\n";
            return 0;
        }

        uint32_t nic_idx = GetNicIdxOfQp(qp);
        Ptr<QbbNetDevice> dev = m_nic[nic_idx].dev;
        if (m_ack_interval == 0)
            std::cout << "ERROR: shouldn't receive ack\n";
        else
        {
            if (!m_backto0)
            {
                qp->Acknowledge(seq);
            }
            else
            {
                uint32_t goback_seq = seq / m_chunk * m_chunk;
                qp->Acknowledge(goback_seq);
            }
            if (qp->IsFinished())
            {
                QpComplete(qp);
            }
        }
        if (ch.l3Prot == 0xFD) // NACK
            RecoverQueue(qp);

        // handle cnp
        if (cnp)
        {
            if (m_cc_mode == 1)
            { // mlx version
                cnp_received_mlx(qp->m_CCEntity);
            }
        }

        if (m_cc_mode == 3)
        {
            HandleAckHp(qp->m_CCEntity, p, ch);
        }
        else if (m_cc_mode == 7)
        {
            HandleAckTimely(qp->m_CCEntity, p, ch);
        }
        else if (m_cc_mode == 8)
        {
            HandleAckDctcp(qp->m_CCEntity, p, ch);
        }
        else if (m_cc_mode == 10)
        {
            HandleAckHpPint(qp->m_CCEntity, p, ch);
        }
        // ACK may advance the on-the-fly window, allowing more packets to send
        dev->TriggerTransmit();
        return 0;
    }

    int RdmaHw::Receive(Ptr<Packet> p, CustomHeader &ch)
    {
        L3Protocol proto = static_cast<L3Protocol>(ch.l3Prot);
        switch (proto)
        {
        case L3Protocol::PROTO_UDP:
        {
            SimpleTuple rxTuple = {
                .sip = ch.dip,
                .dip = ch.sip,
                .sport = ch.udp.dport,
                .dport = ch.udp.sport,
                .prio = ch.udp.pg,
            };
            Ptr<RdmaQueuePair> rxQp = GetQp(rxTuple);
            if (rxQp->m_connectionAttr.qp_type == QPType::RDMA_RC)
            {
                RCReceiveUdp(p, rxQp, ch);
            }
            else
            {
                UCReceiveUdp(p, rxQp, ch);
            }
        }
        /*
        else {

        }
        */
        break;
        case L3Protocol::PROTO_NACK:
            ReceiveAck(p, ch);
            break;
        case L3Protocol::PROTO_ACK:
            ReceiveAck(p, ch);
            break;
        } // namespace ns3

        return 0;
    }

    RCSeqState RdmaHw::RCReceiverCheckSeq(uint32_t seq, Ptr<RdmaQueuePair> q, uint32_t size)
    {
        uint32_t expected = q->ReceiverNextExpectedSeq;
        if (seq == expected)
        {
            q->ReceiverNextExpectedSeq = expected + size;
            if (q->ReceiverNextExpectedSeq >= q->m_milestone_rx)
            {
                q->m_milestone_rx += m_ack_interval;
                return RCSeqState::GENERATE_ACK; // Generate ACK
            }
            else if (q->ReceiverNextExpectedSeq % m_chunk == 0)
            {
                return RCSeqState::GENERATE_ACK;
            }
            else
            {
                return RCSeqState::NOT_GENERATE_ACK;
            }
        }
        else if (seq > expected)
        {
            // Generate NACK
            if (Simulator::Now() >= q->m_nackTimer || q->m_lastNACK != expected)
            {
                q->m_nackTimer = Simulator::Now() + MicroSeconds(m_nack_interval);
                q->m_lastNACK = expected;
                if (m_backto0)
                {
                    q->ReceiverNextExpectedSeq = q->ReceiverNextExpectedSeq / m_chunk * m_chunk;
                }
                return RCSeqState::GENERATE_NACK;
            }
            else
                return RCSeqState::NOT_GENERATE_NACK;
        }
        else
        {
            // Duplicate.
            return RCSeqState::DUPLICATED;
        }
    }

    UCSeqState RdmaHw::UCReceiverCheckSeq(CustomHeader &header, Ptr<RdmaQueuePair> q, uint32_t size)
    {
        uint32_t seq = header.udp.seq;
        uint32_t expected = q->ReceiverNextExpectedSeq;

        if (seq < expected)
        {
            // silently dropped
            return UCSeqState::DUPLICATED;
        }

        if (seq > expected)
        {
            q->ReceiverNextExpectedSeq = seq;
            /*
        switch (header.udp.ibh.GetOpCode().GetOpCodeOperation()) {
            case OpCodeOperation::SEND_FIRST:;
            case OpCodeOperation::SEND_LAST_WITH_IMM:
            case OpCodeOperation::SEND_ONLY_WITH_IMM:;
            case OpCodeOperation::SEND_LAST:
            case OpCodeOperation::SEND_ONLY:
                NS_ASSERT_MSG(false, " INVALID OP CODE");
        }
        */
            return UCSeqState::OOS;
        }
        /*
    if (seq == exp) {
        return UCSeqState::OK;
    }
    */
        q->ReceiverNextExpectedSeq;
        /*
    if (seq == expected) {
        q->ReceiverNextExpectedSeq = expected + size;
        if (q->ReceiverNextExpectedSeq >= q->m_milestone_rx) {
            q->m_milestone_rx += m_ack_interval;
            return UCSeqState::GENERATE_ACK;  // Generate ACK
        } else if (q->ReceiverNextExpectedSeq % m_chunk == 0) {
            return UCSeqState::GENERATE_ACK;
        } else {

            return CheckSeqState::NOT_GENERATE_ACK;
        }
    } else if (seq > expected) {
        // Generate NACK
        if (Simulator::Now() >= q->m_nackTimer || q->m_lastNACK != expected) {
            q->m_nackTimer = Simulator::Now() + MicroSeconds(m_nack_interval);
            q->m_lastNACK = expected;
            if (m_backto0) {
                q->ReceiverNextExpectedSeq = q->ReceiverNextExpectedSeq / m_chunk * m_chunk;
            }
            return CheckSeqState::GENERATE_NACK;
        } else
            return CheckSeqState::NOT_GENERATE_NACK;
    } else {
        // Duplicate.
        return CheckSeqState::DUPLICATED;
    }
    */
    }

    void RdmaHw::AddHeader(Ptr<Packet> p, uint16_t protocolNumber)
    {
        PppHeader ppp;
        ppp.SetProtocol(EtherToPpp(protocolNumber));
        p->AddHeader(ppp);
    }

    uint16_t RdmaHw::EtherToPpp(uint16_t proto)
    {
        switch (proto)
        {
        case 0x0800:
            return 0x0021; // IPv4
        case 0x86DD:
            return 0x0057; // IPv6
        default:
            NS_ASSERT_MSG(false, "PPP Protocol number not defined!");
        }
        return 0;
    }

    void RdmaHw::RecoverQueue(Ptr<RdmaQueuePair> qp) { qp->snd_nxt = qp->snd_una; }

    void RdmaHw::QpComplete(Ptr<RdmaQueuePair> qp)
    {
        NS_ASSERT(!m_qpCompleteCallback.IsNull());
        if (m_cc_mode == 1)
        {
            Simulator::Cancel(qp->m_CCEntity->mlx.m_eventUpdateAlpha);
            Simulator::Cancel(qp->m_CCEntity->mlx.m_eventDecreaseRate);
            Simulator::Cancel(qp->m_CCEntity->mlx.m_rpTimer);
        }

        // This callback will log info
        // It may also delete the rxQp on the receiver
        m_qpCompleteCallback(qp);

        qp->m_notifyAppFinish();

        // delete the qp
        DeleteQueuePair(qp);
    }

    void RdmaHw::SetLinkDown(Ptr<QbbNetDevice> dev) { printf("RdmaHw: node:%u a link down\n", m_node->GetId()); }

    void RdmaHw::AddTableEntry(Ipv4Address &dstAddr, uint32_t intf_idx)
    {
        uint32_t dip = dstAddr.Get();
        m_rtTable[dip].push_back(intf_idx);
    }

    void RdmaHw::ClearTable() { m_rtTable.clear(); }

    void RdmaHw::RedistributeQp()
    {
        // clear old qpGrp
        for (uint32_t i = 0; i < m_nic.size(); i++)
        {
            if (m_nic[i].dev == NULL)
                continue;
            m_nic[i].qpGrp->Clear();
        }

        // redistribute qp
        for (auto &it : m_qpMap)
        {
            Ptr<RdmaQueuePair> qp = it.second;
            uint32_t nic_idx = GetNicIdxOfQp(qp);
            m_nic[nic_idx].qpGrp->AddQp(qp);
            // Notify Nic
            m_nic[nic_idx].dev->ReassignedQp(qp);
        }
    }

    Ptr<Packet> RdmaHw::GetNxtPacket(Ptr<RdmaQueuePair> qp)
    {

        uint32_t payload_size = qp->GetBytesLeft();
        if (m_mtu < payload_size)
            payload_size = m_mtu;
        Ptr<Packet> p = Create<Packet>(payload_size);
        // add SeqTsHeader
        SeqTsHeader seqTs;
        seqTs.SetSeq(qp->snd_nxt);
        seqTs.SetPG(qp->m_connectionAttr.pg);
        p->AddHeader(seqTs);
        // add udp header
        UdpHeader udpHeader;
        udpHeader.SetDestinationPort(qp->m_connectionAttr.dport);
        udpHeader.SetSourcePort(qp->m_connectionAttr.sport);
        p->AddHeader(udpHeader);
        // add ipv4 header
        Ipv4Header ipHeader;
        ipHeader.SetSource(qp->m_connectionAttr.sip);
        ipHeader.SetDestination(qp->m_connectionAttr.dip);
        ipHeader.SetProtocol(0x11);
        ipHeader.SetPayloadSize(p->GetSize());
        ipHeader.SetTtl(64);
        ipHeader.SetTos(0);
        ipHeader.SetIdentification(qp->m_ipid);
        p->AddHeader(ipHeader);
        // add ppp header
        PppHeader ppp;
        ppp.SetProtocol(0x0021); // EtherToPpp(0x800), see point-to-point-net-device.cc
        p->AddHeader(ppp);

        // update state
        qp->snd_nxt += payload_size;
        qp->m_ipid++;

        // return
        return p;
    }

    void RdmaHw::PktSent(Ptr<RdmaQueuePair> qp, Ptr<Packet> pkt, Time interframeGap)
    {
        qp->m_CCEntity->lastPktSize = pkt->GetSize();
        UpdateNextAvail(qp, interframeGap, pkt->GetSize());
    }

    void RdmaHw::UpdateNextAvail(Ptr<RdmaQueuePair> qp, Time interframeGap, uint32_t pkt_size)
    {
        Time sendingTime;
        if (m_rateBound)
            sendingTime = interframeGap + Seconds(qp->m_CCEntity->m_rate.CalculateTxTime(pkt_size));
        else
            sendingTime = interframeGap + Seconds(qp->m_CCEntity->m_max_rate.CalculateTxTime(pkt_size));
        qp->m_nextAvail = Simulator::Now() + sendingTime;
    }

    void RdmaHw::ChangeRate(Ptr<CongestionControlEntity> qp, DataRate new_rate)
    {
#if 1
        Time sendingTime = Seconds(qp->m_rate.CalculateTxTime(qp->lastPktSize));
        Time new_sendintTime = Seconds(new_rate.CalculateTxTime(qp->lastPktSize));

        // update nic's next avail event
        // TO DO Krayecho Yx: fix me: bug for QP distributed on NICs
        //qp->m_nextAvail = qp->m_nextAvail + new_sendintTime - sendingTime;
        //uint32_t nic_idx = GetNicIdxOfQp(qp);
        //m_nic[nic_idx].dev->UpdateNextAvail(qp->m_nextAvail);
#endif

        // change to new rate
        qp->m_rate = new_rate;
    }

#define PRINT_LOG 0
    /******************************
 * Mellanox's version of DCQCN
 *****************************/
    void RdmaHw::UpdateAlphaMlx(Ptr<CongestionControlEntity> q)
    {
#if PRINT_LOG
// std::cout << Simulator::Now() << " alpha update:" << m_node->GetId() << ' ' << q->mlx.m_alpha << ' ' << (int)q->mlx.m_alpha_cnp_arrived << '\n';
// printf("%lu alpha update: %08x %08x %u %u %.6lf->", Simulator::Now().GetTimeStep(), q->m_connectionAttr.sip.Get(), q->m_connectionAttr.dip.Get(),
// q->m_connectionAttr.sport, q->m_connectionAttr.dport, q->mlx.m_alpha);
#endif
        if (q->mlx.m_alpha_cnp_arrived)
        {
            q->mlx.m_alpha = (1 - m_g) * q->mlx.m_alpha + m_g; // binary feedback
        }
        else
        {
            q->mlx.m_alpha = (1 - m_g) * q->mlx.m_alpha; // binary feedback
        }
#if PRINT_LOG
// printf("%.6lf\n", q->mlx.m_alpha);
#endif
        q->mlx.m_alpha_cnp_arrived = false; // clear the CNP_arrived bit
        ScheduleUpdateAlphaMlx(q);
    }
    void RdmaHw::ScheduleUpdateAlphaMlx(Ptr<CongestionControlEntity> q)
    {
        q->mlx.m_eventUpdateAlpha = Simulator::Schedule(MicroSeconds(m_alpha_resume_interval), &RdmaHw::UpdateAlphaMlx, this, q);
    }

    void RdmaHw::cnp_received_mlx(Ptr<CongestionControlEntity> q)
    {
        q->mlx.m_alpha_cnp_arrived = true;    // set CNP_arrived bit for alpha update
        q->mlx.m_decrease_cnp_arrived = true; // set CNP_arrived bit for rate decrease
        if (q->mlx.m_first_cnp)
        {
            // init alpha
            q->mlx.m_alpha = 1;
            q->mlx.m_alpha_cnp_arrived = false;
            // schedule alpha update
            ScheduleUpdateAlphaMlx(q);
            // schedule rate decrease
            ScheduleDecreaseRateMlx(q, 1); // add 1 ns to make sure rate decrease is after alpha update
            // set rate on first CNP
            q->mlx.m_targetRate = q->m_rate = m_rateOnFirstCNP * q->m_rate;
            q->mlx.m_first_cnp = false;
        }
    }

    void RdmaHw::CheckRateDecreaseMlx(Ptr<CongestionControlEntity> q)
    {
        ScheduleDecreaseRateMlx(q, 0);
        if (q->mlx.m_decrease_cnp_arrived)
        {
#if PRINT_LOG
            printf("%lu rate dec: %08x %08x %u %u (%0.3lf %.3lf)->", Simulator::Now().GetTimeStep(), q->m_connectionAttr.sip.Get(),
                   q->m_connectionAttr.dip.Get(), q->m_connectionAttr.sport, q->m_connectionAttr.dport, q->mlx.m_targetRate.GetBitRate() * 1e-9,
                   q->m_rate.GetBitRate() * 1e-9);
#endif
            bool clamp = true;
            if (!m_EcnClampTgtRate)
            {
                if (q->mlx.m_rpTimeStage == 0)
                    clamp = false;
            }
            if (clamp)
                q->mlx.m_targetRate = q->m_rate;
            q->m_rate = std::max(m_minRate, q->m_rate * (1 - q->mlx.m_alpha / 2));
            // reset rate increase related things
            q->mlx.m_rpTimeStage = 0;
            q->mlx.m_decrease_cnp_arrived = false;
            Simulator::Cancel(q->mlx.m_rpTimer);
            q->mlx.m_rpTimer = Simulator::Schedule(MicroSeconds(m_rpgTimeReset), &RdmaHw::RateIncEventTimerMlx, this, q);
#if PRINT_LOG
            printf("(%.3lf %.3lf)\n", q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
        }
    }
    void RdmaHw::ScheduleDecreaseRateMlx(Ptr<CongestionControlEntity> q, uint32_t delta)
    {
        q->mlx.m_eventDecreaseRate =
            Simulator::Schedule(MicroSeconds(m_rateDecreaseInterval) + NanoSeconds(delta), &RdmaHw::CheckRateDecreaseMlx, this, q);
    }

    void RdmaHw::RateIncEventTimerMlx(Ptr<CongestionControlEntity> q)
    {
        q->mlx.m_rpTimer = Simulator::Schedule(MicroSeconds(m_rpgTimeReset), &RdmaHw::RateIncEventTimerMlx, this, q);
        RateIncEventMlx(q);
        q->mlx.m_rpTimeStage++;
    }
    void RdmaHw::RateIncEventMlx(Ptr<CongestionControlEntity> q)
    {
        // check which increase phase: fast recovery, active increase, hyper increase
        if (q->mlx.m_rpTimeStage < m_rpgThreshold)
        { // fast recovery
            FastRecoveryMlx(q);
        }
        else if (q->mlx.m_rpTimeStage == m_rpgThreshold)
        { // active increase
            ActiveIncreaseMlx(q);
        }
        else
        { // hyper increase
            HyperIncreaseMlx(q);
        }
    }

    void RdmaHw::FastRecoveryMlx(Ptr<CongestionControlEntity> q)
    {
#if PRINT_LOG
        printf("%lu fast recovery: %08x %08x %u %u (%0.3lf %.3lf)->", Simulator::Now().GetTimeStep(), q->m_connectionAttr.sip.Get(),
               q->m_connectionAttr.dip.Get(), q->m_connectionAttr.sport, q->m_connectionAttr.dport, q->mlx.m_targetRate.GetBitRate() * 1e-9,
               q->m_rate.GetBitRate() * 1e-9);
#endif
        q->m_rate = (q->m_rate / 2) + (q->mlx.m_targetRate / 2);
#if PRINT_LOG
        printf("(%.3lf %.3lf)\n", q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
    }
    void RdmaHw::ActiveIncreaseMlx(Ptr<CongestionControlEntity> q)
    {
#if PRINT_LOG
        printf("%lu active inc: %08x %08x %u %u (%0.3lf %.3lf)->", Simulator::Now().GetTimeStep(), q->m_connectionAttr.sip.Get(),
               q->m_connectionAttr.dip.Get(), q->m_connectionAttr.sport, q->m_connectionAttr.dport, q->mlx.m_targetRate.GetBitRate() * 1e-9,
               q->m_rate.GetBitRate() * 1e-9);
#endif
        // get NIC
        // increate rate
        q->mlx.m_targetRate += m_rai;
        if (q->mlx.m_targetRate > q->m_max_rate)
            q->mlx.m_targetRate = q->m_max_rate;
        q->m_rate = (q->m_rate / 2) + (q->mlx.m_targetRate / 2);
#if PRINT_LOG
        printf("(%.3lf %.3lf)\n", q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
    }
    void RdmaHw::HyperIncreaseMlx(Ptr<CongestionControlEntity> q)
    {
#if PRINT_LOG
        printf("%lu hyper inc: %08x %08x %u %u (%0.3lf %.3lf)->", Simulator::Now().GetTimeStep(), q->m_connectionAttr.sip.Get(),
               q->m_connectionAttr.dip.Get(), q->m_connectionAttr.sport, q->m_connectionAttr.dport, q->mlx.m_targetRate.GetBitRate() * 1e-9,
               q->m_rate.GetBitRate() * 1e-9);
#endif
        // increate rate
        q->mlx.m_targetRate += m_rhai;
        if (q->mlx.m_targetRate > q->m_max_rate)
            q->mlx.m_targetRate = q->m_max_rate;
        q->m_rate = (q->m_rate / 2) + (q->mlx.m_targetRate / 2);
#if PRINT_LOG
        printf("(%.3lf %.3lf)\n", q->mlx.m_targetRate.GetBitRate() * 1e-9, q->m_rate.GetBitRate() * 1e-9);
#endif
    }

    /***********************
 * High Precision CC
 ***********************/
    void RdmaHw::HandleAckHp(Ptr<CongestionControlEntity> qp, Ptr<Packet> p, CustomHeader &ch)
    {
        uint32_t ack_seq = ch.ack.seq;
        // update rate
        if (ack_seq > qp->hp.m_lastUpdateSeq)
        { // if full RTT feedback is ready, do full update
            UpdateRateHp(qp, p, ch, false);
        }
        else
        { // do fast react
            FastReactHp(qp, p, ch);
        }
    }

    void RdmaHw::UpdateRateHp(Ptr<CongestionControlEntity> sender, Ptr<Packet> p, CustomHeader &ch, bool fast_react)
    {
        Ptr<RdmaQueuePair> qp = DynamicCast<RdmaQueuePair, CongestionControlEntity>(sender);
        uint32_t next_seq = qp->snd_nxt;
        bool print = !fast_react || true;
        if (sender->hp.m_lastUpdateSeq == 0)
        { // first RTT
            sender->hp.m_lastUpdateSeq = next_seq;
            // store INT
            IntHeader &ih = ch.ack.ih;
            NS_ASSERT(ih.nhop <= IntHeader::maxHop);
            for (uint32_t i = 0; i < ih.nhop; i++)
                sender->hp.hop[i] = ih.hop[i];
#if PRINT_LOG
            if (print)
            {
                printf("%lu %s %08x %08x %u %u [%u,%u,%u]", Simulator::Now().GetTimeStep(), fast_react ? "fast" : "update",
                       qp->m_connectionAttr.sip.Get(), qp->m_connectionAttr.dip.Get(), qp->m_connectionAttr.sport, qp->m_connectionAttr.dport,
                       qp->hp.m_lastUpdateSeq, ch.ack.seq, next_seq);
                for (uint32_t i = 0; i < ih.nhop; i++)
                    printf(" %u %lu %lu", ih.hop[i].GetQlen(), ih.hop[i].GetBytes(), ih.hop[i].GetTime());
                printf("\n");
            }
#endif
        }
        else
        {
            // check packet INT
            IntHeader &ih = ch.ack.ih;
            if (ih.nhop <= IntHeader::maxHop)
            {
                double max_c = 0;
                bool inStable = false;
#if PRINT_LOG
                if (print)
                    printf("%lu %s %08x %08x %u %u [%u,%u,%u]", Simulator::Now().GetTimeStep(), fast_react ? "fast" : "update",
                           qp->m_connectionAttr.sip.Get(), qp->m_connectionAttr.dip.Get(), qp->m_connectionAttr.sport, qp->m_connectionAttr.dport,
                           qp->hp.m_lastUpdateSeq, ch.ack.seq, next_seq);
#endif
                // check each hop
                double U = 0;
                uint64_t dt = 0;
                bool updated[IntHeader::maxHop] = {false}, updated_any = false;
                NS_ASSERT(ih.nhop <= IntHeader::maxHop);
                for (uint32_t i = 0; i < ih.nhop; i++)
                {
                    if (m_sampleFeedback)
                    {
                        if (ih.hop[i].GetQlen() == 0 and fast_react)
                            continue;
                    }
                    updated[i] = updated_any = true;
#if PRINT_LOG
                    if (print)
                        printf(" %u(%u) %lu(%lu) %lu(%lu)", ih.hop[i].GetQlen(), sender->hp.hop[i].GetQlen(), ih.hop[i].GetBytes(), sender->hp.hop[i].GetBytes(),
                               ih.hop[i].GetTime(), sender->hp.hop[i].GetTime());
#endif
                    uint64_t tau = ih.hop[i].GetTimeDelta(sender->hp.hop[i]);
                    ;
                    double duration = tau * 1e-9;
                    double txRate = (ih.hop[i].GetBytesDelta(sender->hp.hop[i])) * 8 / duration;
                    double u = txRate / ih.hop[i].GetLineRate() + (double)std::min(ih.hop[i].GetQlen(), sender->hp.hop[i].GetQlen()) *
                                                                      sender->m_max_rate.GetBitRate() / ih.hop[i].GetLineRate() / sender->m_win;
#if PRINT_LOG
                    if (print)
                        printf(" %.3lf %.3lf", txRate, u);
#endif
                    if (!m_multipleRate)
                    {
                        // for aggregate (single R)
                        if (u > U)
                        {
                            U = u;
                            dt = tau;
                        }
                    }
                    else
                    {
                        // for per hop (per hop R)
                        if (tau > sender->m_baseRtt)
                            tau = sender->m_baseRtt;
                        sender->hp.hopState[i].u = (sender->hp.hopState[i].u * (sender->m_baseRtt - tau) + u * tau) / double(sender->m_baseRtt);
                    }
                    sender->hp.hop[i] = ih.hop[i];
                }

                DataRate new_rate;
                int32_t new_incStage;
                DataRate new_rate_per_hop[IntHeader::maxHop];
                int32_t new_incStage_per_hop[IntHeader::maxHop];
                if (!m_multipleRate)
                {
                    // for aggregate (single R)
                    if (updated_any)
                    {
                        if (dt > sender->m_baseRtt)
                            dt = sender->m_baseRtt;
                        sender->hp.u = (sender->hp.u * (sender->m_baseRtt - dt) + U * dt) / double(sender->m_baseRtt);
                        max_c = sender->hp.u / m_targetUtil;

                        if (max_c >= 1 || sender->hp.m_incStage >= m_miThresh)
                        {
                            new_rate = sender->hp.m_curRate / max_c + m_rai;
                            new_incStage = 0;
                        }
                        else
                        {
                            new_rate = sender->hp.m_curRate + m_rai;
                            new_incStage = sender->hp.m_incStage + 1;
                        }
                        if (new_rate < m_minRate)
                            new_rate = m_minRate;
                        if (new_rate > sender->m_max_rate)
                            new_rate = sender->m_max_rate;
#if PRINT_LOG
                        if (print)
                            printf(" u=%.6lf U=%.3lf dt=%u max_c=%.3lf", sender->hp.u, U, dt, max_c);
#endif
#if PRINT_LOG
                        if (print)
                            printf(" rate:%.3lf->%.3lf\n", sender->hp.m_curRate.GetBitRate() * 1e-9, new_rate.GetBitRate() * 1e-9);
#endif
                    }
                }
                else
                {
                    // for per hop (per hop R)
                    new_rate = sender->m_max_rate;
                    for (uint32_t i = 0; i < ih.nhop; i++)
                    {
                        if (updated[i])
                        {
                            double c = sender->hp.hopState[i].u / m_targetUtil;
                            if (c >= 1 || sender->hp.hopState[i].incStage >= m_miThresh)
                            {
                                new_rate_per_hop[i] = sender->hp.hopState[i].Rc / c + m_rai;
                                new_incStage_per_hop[i] = 0;
                            }
                            else
                            {
                                new_rate_per_hop[i] = sender->hp.hopState[i].Rc + m_rai;
                                new_incStage_per_hop[i] = sender->hp.hopState[i].incStage + 1;
                            }
                            // bound rate
                            if (new_rate_per_hop[i] < m_minRate)
                                new_rate_per_hop[i] = m_minRate;
                            if (new_rate_per_hop[i] > sender->m_max_rate)
                                new_rate_per_hop[i] = sender->m_max_rate;
                            // find min new_rate
                            if (new_rate_per_hop[i] < new_rate)
                                new_rate = new_rate_per_hop[i];
#if PRINT_LOG
                            if (print)
                                printf(" [%u]u=%.6lf c=%.3lf", i, sender->hp.hopState[i].u, c);
#endif
#if PRINT_LOG
                            if (print)
                                printf(" %.3lf->%.3lf", sender->hp.hopState[i].Rc.GetBitRate() * 1e-9, new_rate.GetBitRate() * 1e-9);
#endif
                        }
                        else
                        {
                            if (sender->hp.hopState[i].Rc < new_rate)
                                new_rate = sender->hp.hopState[i].Rc;
                        }
                    }
#if PRINT_LOG
                    printf("\n");
#endif
                }
                if (updated_any)
                    ChangeRate(sender, new_rate);
                if (!fast_react)
                {
                    if (updated_any)
                    {
                        sender->hp.m_curRate = new_rate;
                        sender->hp.m_incStage = new_incStage;
                    }
                    if (m_multipleRate)
                    {
                        // for per hop (per hop R)
                        for (uint32_t i = 0; i < ih.nhop; i++)
                        {
                            if (updated[i])
                            {
                                sender->hp.hopState[i].Rc = new_rate_per_hop[i];
                                sender->hp.hopState[i].incStage = new_incStage_per_hop[i];
                            }
                        }
                    }
                }
            }
            if (!fast_react)
            {
                if (next_seq > sender->hp.m_lastUpdateSeq)
                    sender->hp.m_lastUpdateSeq = next_seq; //+ rand() % 2 * m_mtu;
            }
        }
    }

    void RdmaHw::FastReactHp(Ptr<CongestionControlEntity> qp, Ptr<Packet> p, CustomHeader &ch)
    {
        if (m_fast_react)
            UpdateRateHp(qp, p, ch, true);
    }

    /**********************
 * TIMELY
 *********************/
    void RdmaHw::HandleAckTimely(Ptr<CongestionControlEntity> qp, Ptr<Packet> p, CustomHeader &ch)
    {
        uint32_t ack_seq = ch.ack.seq;
        // update rate
        if (ack_seq > qp->tmly.m_lastUpdateSeq)
        { // if full RTT feedback is ready, do full update
            UpdateRateTimely(qp, p, ch, false);
        }
        else
        { // do fast react
            FastReactTimely(qp, p, ch);
        }
    }
    void RdmaHw::UpdateRateTimely(Ptr<CongestionControlEntity> sender, Ptr<Packet> p, CustomHeader &ch, bool us)
    {
        Ptr<RdmaQueuePair> qp = DynamicCast<RdmaQueuePair, CongestionControlEntity>(sender);
        uint32_t next_seq = qp->snd_nxt;
        uint64_t rtt = Simulator::Now().GetTimeStep() - ch.ack.ih.ts;
        bool print = !us;
        if (sender->tmly.m_lastUpdateSeq != 0)
        { // not first RTT
            int64_t new_rtt_diff = (int64_t)rtt - (int64_t)sender->tmly.lastRtt;
            double rtt_diff = (1 - m_tmly_alpha) * sender->tmly.rttDiff + m_tmly_alpha * new_rtt_diff;
            double gradient = rtt_diff / m_tmly_minRtt;
            bool inc = false;
            double c = 0;
#if PRINT_LOG
            if (print)
                printf("%lu node:%u rtt:%lu rttDiff:%.0lf gradient:%.3lf rate:%.3lf", Simulator::Now().GetTimeStep(), m_node->GetId(), rtt, rtt_diff,
                       gradient, sender->tmly.m_curRate.GetBitRate() * 1e-9);
#endif
            if (rtt < m_tmly_TLow)
            {
                inc = true;
            }
            else if (rtt > m_tmly_THigh)
            {
                c = 1 - m_tmly_beta * (1 - (double)m_tmly_THigh / rtt);
                inc = false;
            }
            else if (gradient <= 0)
            {
                inc = true;
            }
            else
            {
                c = 1 - m_tmly_beta * gradient;
                if (c < 0)
                    c = 0;
                inc = false;
            }
            if (inc)
            {
                if (sender->tmly.m_incStage < 5)
                {
                    sender->m_rate = sender->tmly.m_curRate + m_rai;
                }
                else
                {
                    sender->m_rate = sender->tmly.m_curRate + m_rhai;
                }
                if (sender->m_rate > sender->m_max_rate)
                    sender->m_rate = sender->m_max_rate;
                if (!us)
                {
                    sender->tmly.m_curRate = sender->m_rate;
                    sender->tmly.m_incStage++;
                    sender->tmly.rttDiff = rtt_diff;
                }
            }
            else
            {
                sender->m_rate = std::max(m_minRate, sender->tmly.m_curRate * c);
                if (!us)
                {
                    sender->tmly.m_curRate = sender->m_rate;
                    sender->tmly.m_incStage = 0;
                    sender->tmly.rttDiff = rtt_diff;
                }
            }
#if PRINT_LOG
            if (print)
            {
                printf(" %c %.3lf\n", inc ? '^' : 'v', sender->m_rate.GetBitRate() * 1e-9);
            }
#endif
        }
        if (!us && next_seq > sender->tmly.m_lastUpdateSeq)
        {
            sender->tmly.m_lastUpdateSeq = next_seq;
            // update
            sender->tmly.lastRtt = rtt;
        }
    }
    void RdmaHw::FastReactTimely(Ptr<CongestionControlEntity> qp, Ptr<Packet> p, CustomHeader &ch) {}

    /**********************
 * DCTCP
 *********************/
    void RdmaHw::HandleAckDctcp(Ptr<CongestionControlEntity> qp, Ptr<Packet> p, CustomHeader &ch)
    {
        Ptr<RdmaQueuePair> real_qp = DynamicCast<RdmaQueuePair, CongestionControlEntity>(qp);
        uint32_t ack_seq = ch.ack.seq;
        uint8_t cnp = (ch.ack.flags >> qbbHeader::FLAG_CNP) & 1;
        bool new_batch = false;

        // update alpha
        qp->dctcp.m_ecnCnt += (cnp > 0);
        if (ack_seq > qp->dctcp.m_lastUpdateSeq)
        { // if full RTT feedback is ready, do alpha update
#if PRINT_LOG
            printf("%lu %s %08x %08x %u %u [%u,%u,%u] %.3lf->", Simulator::Now().GetTimeStep(), "alpha", real_qp->m_connectionAttr.sip.Get(),
                   real_qp->m_connectionAttr.dip.Get(), real_qp->m_connectionAttr.sport, real_qp->m_connectionAttr.dport, real_qp->dctcp.m_lastUpdateSeq, ch.ack.seq,
                   qp->snd_nxt, qp->dctcp.m_alpha);
#endif
            new_batch = true;
            if (qp->dctcp.m_lastUpdateSeq == 0)
            { // first RTT
                qp->dctcp.m_lastUpdateSeq = real_qp->snd_nxt;
                qp->dctcp.m_batchSizeOfAlpha = real_qp->snd_nxt / m_mtu + 1;
            }
            else
            {
                double frac = std::min(1.0, double(qp->dctcp.m_ecnCnt) / qp->dctcp.m_batchSizeOfAlpha);
                qp->dctcp.m_alpha = (1 - m_g) * qp->dctcp.m_alpha + m_g * frac;
                qp->dctcp.m_lastUpdateSeq = real_qp->snd_nxt;
                qp->dctcp.m_ecnCnt = 0;
                qp->dctcp.m_batchSizeOfAlpha = (real_qp->snd_nxt - ack_seq) / m_mtu + 1;
#if PRINT_LOG
                printf("%.3lf F:%.3lf", qp->dctcp.m_alpha, frac);
#endif
            }
#if PRINT_LOG
            printf("\n");
#endif
        }

        // check cwr exit
        if (qp->dctcp.m_caState == 1)
        {
            if (ack_seq > qp->dctcp.m_highSeq)
                qp->dctcp.m_caState = 0;
        }

        // check if need to reduce rate: ECN and not in CWR
        if (cnp && qp->dctcp.m_caState == 0)
        {
#if PRINT_LOG
            printf("%lu %s %08x %08x %u %u %.3lf->", Simulator::Now().GetTimeStep(), "rate", qp->m_connectionAttr.sip.Get(),
                   qp->m_connectionAttr.dip.Get(), qp->m_connectionAttr.sport, qp->m_connectionAttr.dport, qp->m_rate.GetBitRate() * 1e-9);
#endif
            qp->m_rate = std::max(m_minRate, qp->m_rate * (1 - qp->dctcp.m_alpha / 2));
#if PRINT_LOG
            printf("%.3lf\n", qp->m_rate.GetBitRate() * 1e-9);
#endif
            qp->dctcp.m_caState = 1;
            qp->dctcp.m_highSeq = real_qp->snd_nxt;
        }

        // additive inc
        if (qp->dctcp.m_caState == 0 && new_batch)
            qp->m_rate = std::min(qp->m_max_rate, qp->m_rate + m_dctcp_rai);
    }

    /*********************
 * HPCC-PINT
 ********************/
    void RdmaHw::SetPintSmplThresh(double p) { pint_smpl_thresh = (uint32_t)(65536 * p); }
    void RdmaHw::HandleAckHpPint(Ptr<CongestionControlEntity> qp, Ptr<Packet> p, CustomHeader &ch)
    {
        uint32_t ack_seq = ch.ack.seq;
        if (rand() % 65536 >= pint_smpl_thresh)
            return;
        // update rate
        if (ack_seq > qp->hpccPint.m_lastUpdateSeq)
        { // if full RTT feedback is ready, do full update
            UpdateRateHpPint(qp, p, ch, false);
        }
        else
        { // do fast react
            UpdateRateHpPint(qp, p, ch, true);
        }
    }

    void RdmaHw::UpdateRateHpPint(Ptr<CongestionControlEntity> qp, Ptr<Packet> p, CustomHeader &ch, bool fast_react)
    {
        Ptr<RdmaQueuePair> real_qp = DynamicCast<RdmaQueuePair, CongestionControlEntity>(qp);
        uint32_t next_seq = real_qp->snd_nxt;
        if (qp->hpccPint.m_lastUpdateSeq == 0)
        { // first RTT
            qp->hpccPint.m_lastUpdateSeq = next_seq;
        }
        else
        {
            // check packet INT
            IntHeader &ih = ch.ack.ih;
            double U = Pint::decode_u(ih.GetPower());

            DataRate new_rate;
            int32_t new_incStage;
            double max_c = U / m_targetUtil;

            if (max_c >= 1 || qp->hpccPint.m_incStage >= m_miThresh)
            {
                new_rate = qp->hpccPint.m_curRate / max_c + m_rai;
                new_incStage = 0;
            }
            else
            {
                new_rate = qp->hpccPint.m_curRate + m_rai;
                new_incStage = qp->hpccPint.m_incStage + 1;
            }
            if (new_rate < m_minRate)
                new_rate = m_minRate;
            if (new_rate > qp->m_max_rate)
                new_rate = qp->m_max_rate;
            ChangeRate(qp, new_rate);
            if (!fast_react)
            {
                qp->hpccPint.m_curRate = new_rate;
                qp->hpccPint.m_incStage = new_incStage;
            }
            if (!fast_react)
            {
                if (next_seq > qp->hpccPint.m_lastUpdateSeq)
                    qp->hpccPint.m_lastUpdateSeq = next_seq; //+ rand() % 2 * m_mtu;
            }
        }
    }

} // namespace ns3
