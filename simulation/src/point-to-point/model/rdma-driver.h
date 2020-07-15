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

using QPCreateAttribute = struct qp_create_attr {
    QPConnectionAttr conAttr;
    uint64_t size;
    uint32_t win;
    uint64_t baseRtt;
    Callback<void> notifyAppFinish;
    Callback<void, IBVWorkCompletion&> notifyCompletion;
    qp_create_attr(const QPConnectionAttr& p_con_attr, uint64_t p_size, uint32_t p_win, uint64_t p_baseRtt, Callback<void> p_notifyAppFinish,
                   Callback<void, IBVWorkCompletion&> p_notifyCompletion);
};

inline QPCreateAttribute::qp_create_attr(const QPConnectionAttr& p_con_attr, uint64_t p_size, uint32_t p_win, uint64_t p_base_rtt,
                                         Callback<void> p_notify_app_finish, Callback<void, IBVWorkCompletion&> p_notify_completion)
    : conAttr(p_con_attr),
      size(p_size),
      win(p_win),
      baseRtt(p_base_rtt),
      notifyAppFinish(p_notify_app_finish),
      notifyCompletion(p_notify_completion){};

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

    // Verbs
    void PostSend(IBVWorkRequest& wr);
    // TO DO Krayecho Yx:
    // void PostReceive();

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
