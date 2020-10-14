#include "rdma-driver.h"

namespace ns3 {

/***********************
 * RdmaDriver
 **********************/
TypeId RdmaDriver::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::RdmaDriver")
                            .SetParent<Object>()
                            .AddTraceSource("QpComplete", "A qp completes.", MakeTraceSourceAccessor(&RdmaDriver::m_traceQpComplete));
    return tid;
}

RdmaDriver::RdmaDriver() {}

void RdmaDriver::Init(void) {
    Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
#if 0
	m_rdma->m_nic.resize(ipv4->GetNInterfaces());
	for (uint32_t i = 0; i < m_rdma->m_nic.size(); i++){
		m_rdma->m_nic[i] = CreateObject<CongestionControlGroup>();
		// share the queue pair group with NIC
		if (ipv4->GetNetDevice(i)->IsQbb()){
			DynamicCast<QbbNetDevice>(ipv4->GetNetDevice(i))->m_rdmaEQ->m_qpGrp = m_rdma->m_nic[i];
		}
	}
#endif
    for (uint32_t i = 0; i < m_node->GetNDevices(); i++) {
        Ptr<QbbNetDevice> dev = NULL;
        if (m_node->GetDevice(i)->IsQbb()) dev = DynamicCast<QbbNetDevice>(m_node->GetDevice(i));
        m_rdma->m_nic.push_back(RdmaInterfaceMgr(dev));
        m_rdma->m_nic.back().qpGrp = CreateObject<QueuePairSet>();
    }
#if 0
	for (uint32_t i = 0; i < ipv4->GetNInterfaces (); i++){
		if (ipv4->GetNetDevice(i)->IsQbb() && ipv4->IsUp(i)){
			Ptr<QbbNetDevice> dev = DynamicCast<QbbNetDevice>(ipv4->GetNetDevice(i));
			// add a new RdmaInterfaceMgr for this device
			m_rdma->m_nic.push_back(RdmaInterfaceMgr(dev));
			m_rdma->m_nic.back().qpGrp = CreateObject<QueuePairSet>();
		}
	}
#endif
    // RdmaHw do setup
    m_rdma->SetNode(m_node);
    m_rdma->Setup(MakeCallback(&RdmaDriver::QpComplete, this));
}

void RdmaDriver::SetNode(Ptr<Node> node) { m_node = node; }

void RdmaDriver::SetRdmaHw(Ptr<RdmaHw> rdma) { m_rdma = rdma; }

Ptr<RdmaQueuePair> RdmaDriver::AddQueuePair(QPCreateAttribute& create_attr) {
    // TO DO Krayecho Yx: converge this
    return m_rdma->AddQueuePair(create_attr.conAttr);
}

void RdmaDriver::QpComplete(Ptr<RdmaQueuePair> q) { m_traceQpComplete(q); }

}  // namespace ns3
