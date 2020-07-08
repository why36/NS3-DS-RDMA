#ifndef RDMA_DRIVER_H
#define RDMA_DRIVER_H

#include <ns3/node.h>
#include <ns3/qbb-net-device.h>
#include <ns3/rdma-hw.h>
#include <ns3/rdma-queue-pair.h>
#include <ns3/rdma.h>

#include <unordered_map>
#include <vector>

namespace ns3 {

class SendCompeltionReturnValue;
class RpcResponse;

using QPCreateAttribute = struct {
    QPConnectionAttr conAttr;
    uint64_t size;
    uint32_t win;
    uint64_t baseRtt;
    QPType type;
    Callback<void> notifyAppFinish;
    Callback;
};

class RdmaDriver : public Object {
   public:
    Ptr<Node> m_node;
    Ptr<RdmaHw> m_rdma;

    // trace
    TracedCallback<Ptr<RdmaQueuePair> > m_traceQpComplete;

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
    void AddQueuePair(QPCreateAttribute& QPCreateAttr);

    void PostSendWithImmData();
    void PostWrite();
    void

        // callback when qp completes
        void
        QpComplete(Ptr<RdmaQueuePair> q);
};

}  // namespace ns3

#endif /* RDMA_DRIVER_H */
