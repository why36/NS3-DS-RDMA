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
    QPType type;
    uint64_t size;
    uint32_t win;
    uint64_t baseRtt;
    Callback<void> notifyAppFinish;
    Callback<void, IBVWorkCompletion&> notifyCompletion;
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

    void PostSend(IBVWorkRequest& wr);

    void OnCompletion(IBVWorkCompletion& wc);
    // Callback back to User-space appllication
    // Callback <>

    void OnSendReceived();

    // callback when qp completes
    void QpComplete(Ptr<RdmaQueuePair> q);

   private:
    void OnSendCompletion(IBVWorkCompletion& wc);
    void OnReceiveCompletion(IBVWorkCompletion& wc);
};

/*
// TO DO Krayecho Yx: should be implemented
void RdmaDriver::PostSend(IBVWorkRequest& wr) { return; };
void RdmaDriver::OnCompletion(IBVWorkCompletion& wc) { return; };
void OnSendReceived() { return; };
void OnSendCompletion(IBVWorkCompletion& wc) { return; };
void OnReceiveCompletion(IBVWorkCompletion& wc) { return; };
*/
}  // namespace ns3

#endif /* RDMA_DRIVER_H */
