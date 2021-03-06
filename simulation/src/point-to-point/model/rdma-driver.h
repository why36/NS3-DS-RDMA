#ifndef RDMA_DRIVER_H
#define RDMA_DRIVER_H

#include <ns3/node.h>
#include <ns3/qbb-net-device.h>
#include <ns3/rdma-hw.h>
#include <ns3/rdma-queue-pair.h>

#include <unordered_map>
#include <vector>

namespace ns3 {

using QPCreateAttribute = struct qp_create_attr {
    QPConnectionAttr conAttr;
    qp_create_attr(const QPConnectionAttr& p_con_attr);
    qp_create_attr(uint16_t p_pg, Ipv4Address p_sip, Ipv4Address p_dip, uint16_t p_sport, uint16_t p_dport, QPType p_qp_type);
};

inline QPCreateAttribute::qp_create_attr(const QPConnectionAttr& p_con_attr) : conAttr(p_con_attr){};
inline QPCreateAttribute::qp_create_attr(uint16_t p_pg, Ipv4Address p_sip, Ipv4Address p_dip, uint16_t p_sport, uint16_t p_dport, QPType p_qp_type)
    : conAttr(p_pg, p_sip, p_dip, p_sport, p_dport, p_qp_type){};

class RdmaDriver : public Object {
   public:
    static TypeId GetTypeId(void);
    RdmaDriver();

    // This function init the m_nic according to the NetDevice
    // So this must be called after all NICs are installed
    void Init(void);

    // add a queue pair
    Ptr<RdmaQueuePair> AddQueuePair(QPCreateAttribute& QPCreateAttr);

    // callback when qp completes
    void QpComplete(Ptr<RdmaQueuePair> q);

    void set_node(Ptr<Node> node);
    void set_rdma_hw(Ptr<RdmaHw> rdma);
    Ptr<RdmaHw> get_rdma_hw();

   private:
    Ptr<Node> m_node;
    Ptr<RdmaHw> m_rdma_hw;

    // trace
    TracedCallback<Ptr<RdmaQueuePair>> m_traceQpComplete;
};

inline void RdmaDriver::set_node(Ptr<Node> node) { m_node = node; }
inline void RdmaDriver::set_rdma_hw(Ptr<RdmaHw> rdma) { m_rdma_hw = rdma; }
inline Ptr<RdmaHw> RdmaDriver::get_rdma_hw() { return m_rdma_hw; };

}  // namespace ns3

#endif /* RDMA_DRIVER_H */
