#ifndef RDMA_DRIVER_H
#define RDMA_DRIVER_H

#include <ns3/node.h>
#include <ns3/qbb-net-device.h>
#include <ns3/rdma-hw.h>
#include <ns3/rdma-queue-pair.h>

#include <unordered_map>
#include <vector>

namespace ns3 {

using QpParam = struct qp_param {
    uint64_t m_size;
    uint32_t m_win;      // bound of on-the-fly packets
    uint64_t m_baseRtt;  // base Rtt
    Callback<void> notifyAppFinish;
    Callback<void, Ptr<IBVWorkCompletion>> notifyCompletion;
    qp_param(uint64_t p_size, uint32_t p_win, int64_t p_base_rtt, Callback<void> p_notifyAppFinish,
             Callback<void, Ptr<IBVWorkCompletion>> p_notifyCompletion = MakeNullCallback<void, Ptr<IBVWorkCompletion>>());
    qp_param& operator=(qp_param& rhs);
};

inline QpParam::qp_param(uint64_t p_size, uint32_t p_win, int64_t p_base_rtt, Callback<void> p_notifyAppFinish,
                         Callback<void, Ptr<IBVWorkCompletion>> p_notifyCompletion)
    : m_size(p_size), m_win(p_win), m_baseRtt(p_base_rtt), notifyAppFinish(p_notifyAppFinish), notifyCompletion(p_notifyCompletion){};

inline QpParam& QpParam::operator=(QpParam& rhs) {
    m_size = rhs.m_size;
    m_win = rhs.m_win;
    m_baseRtt = rhs.m_baseRtt;
    notifyAppFinish = rhs.notifyAppFinish;
    notifyCompletion = rhs.notifyCompletion;
}

using QPCreateAttribute = struct qp_create_attr {
    QPConnectionAttr conAttr;
    QpParam qpParam;
    qp_create_attr(const QPConnectionAttr& p_con_attr, const QpParam& param);
    qp_create_attr(uint16_t p_pg, Ipv4Address p_sip, Ipv4Address p_dip, uint16_t p_sport, uint16_t p_dport, QPType p_qp_type, uint64_t p_size,
                   uint32_t p_win, uint64_t p_baseRtt, Callback<void> p_notifyAppFinish,
                   Callback<void, Ptr<IBVWorkCompletion>> p_notifyCompletion = MakeNullCallback<void, Ptr<IBVWorkCompletion>>());
};

inline QPCreateAttribute::qp_create_attr(const QPConnectionAttr& p_con_attr, const QpParam& p_param) : conAttr(p_con_attr), qpParam(p_param){};
inline QPCreateAttribute::qp_create_attr(uint16_t p_pg, Ipv4Address p_sip, Ipv4Address p_dip, uint16_t p_sport, uint16_t p_dport, QPType p_qp_type,
                                         uint64_t p_size, uint32_t p_win, uint64_t p_base_rtt, Callback<void> p_notify_app_finish,
                                         Callback<void, Ptr<IBVWorkCompletion>> p_notify_completion)
    : conAttr(p_pg, p_sip, p_dip, p_sport, p_dport, p_qp_type), qpParam(p_size, p_win, p_base_rtt, p_notify_app_finish, p_notify_completion){};

class RdmaDriver : public Object {
   public:
    Ptr<Node> m_node;
    Ptr<RdmaHw> m_rdma;

    // trace
    TracedCallback<Ptr<RdmaQueuePair>> m_traceQpComplete;

    static TypeId GetTypeId(void);
    RdmaDriver();

    // This function init the m_nic according to the NetDevice
    // So this must be called after all NICs are installed
    void Init(void);

    // Set Node
    void SetNode(Ptr<Node> node);

    // Set RdmaHw
    void SetRdmaHw(Ptr<RdmaHw> rdma);

    // add a queue pair
    Ptr<RdmaQueuePair> AddQueuePair(QPCreateAttribute& QPCreateAttr);

    // callback when qp completes
    void QpComplete(Ptr<RdmaQueuePair> q);

   private:
};

}  // namespace ns3

#endif /* RDMA_DRIVER_H */
