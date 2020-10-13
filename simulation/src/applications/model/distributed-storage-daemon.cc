/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */

/*
 * author:   Yixiao(Krayecho) Gao <532820040@qq.com>
 * date:     202000707
 */

#include "ns3/distributed-storage-daemon.h"

#include <ns3/rdma-driver.h>
#include <stdio.h>
#include <stdlib.h>

#include "ns3/assert.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-end-point.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/qbb-net-device.h"
#include "ns3/random-variable.h"
#include "ns3/rdma-app.h"
#include "ns3/reliability.h"
#include "ns3/rpc.h"
#include "ns3/seq-ts-header.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "ns3/verb-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DistributedStorageDaemon");
NS_OBJECT_ENSURE_REGISTERED(DistributedStorageDaemon);
NS_OBJECT_ENSURE_REGISTERED(DistributedStorageThread);


void DistributedStorageThread::Start(){
    for(auto client:m_clients){
        client->Start();
    }
};

void DistributedStorageThread::AddRPCClient(Ptr<RPCClient> client){
    m_clients.push_back(client);
}
void DistributedStorageThread::AddRPCServer(Ptr<RPCServer> server){
    m_servers.push_back(server);
}

TypeId DistributedStorageDaemon::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::DistributedStorageDaemon").AddConstructor<DistributedStorageDaemon>();
    return tid;
}

DistributedStorageDaemon::DistributedStorageDaemon() { NS_LOG_FUNCTION_NOARGS(); }

DistributedStorageDaemon::~DistributedStorageDaemon() { NS_LOG_FUNCTION_NOARGS(); }

void DistributedStorageDaemon::Connect(Ptr<DistributedStorageDaemon> client, uint32_t client_thread_index, Ptr<DistributedStorageDaemon> server,uint32_t server_thread_index,uint16_t pg) {
    Ptr<DistributedStorageThread> client_thread = client->GetThread(client_thread_index);
    
    uint16_t sport = client->GetNextAvailablePort();
    uint16_t dport = server->GetNextAvailablePort();
    NS_ASSERT(sport && dport);

    Ptr<Node> client_node = client->GetNode();
    Ptr<UserSpaceConnection> srcConnection = Create<UserSpaceConnection>();
    Ptr<RdmaAppQP> srcRdmaAppQP = Create<RdmaAppQP>(client_node->GetObject<RdmaDriver>(),
                          MakeCallback(&UserSpaceConnection::OnTxIBVWC, GetPointer(srcConnection)),
                          MakeCallback(&UserSpaceConnection::OnRxIBVWC, GetPointer(srcConnection)));
    srcConnection->m_appQP = srcRdmaAppQP;
    srcRdmaAppQP->setUSC(srcConnection);
    Ptr<RPCClient> rpc_client = DynamicCast<RPCClient,KRPCClient>(Create<KRPCClient>());
    rpc_client->SetUSC(srcConnection);
    client->GetThread(client_thread_index)->AddRPCClient(rpc_client);



    Ptr<Node> server_node = server->GetNode();
    Ptr<UserSpaceConnection> dstConnection = Create<UserSpaceConnection>();
    Ptr<RdmaAppQP> dstRdmaAppQP = Create<RdmaAppQP>(server_node->GetObject<RdmaDriver>(),
                          MakeCallback(&UserSpaceConnection::OnTxIBVWC, GetPointer(dstConnection)),
                          MakeCallback(&UserSpaceConnection::OnRxIBVWC, GetPointer(dstConnection)));
    // Bind with each other
    dstConnection->m_appQP = dstRdmaAppQP;
    dstRdmaAppQP->setUSC(dstConnection);
    

    Ptr<RPCServer> rpc_server = DynamicCast<RPCServer,KRPCServer>(Create<KRPCServer>());
    rpc_server->SetUSC(srcConnection);
    server->GetThread(server_thread_index)->AddRPCServer(rpc_server);

    QPConnectionAttr srcConnAttr(pg, client->m_ip, server->m_ip, sport, dport, QPType::RDMA_RC);
    RdmaCM::Connect(srcRdmaAppQP, dstRdmaAppQP, srcConnAttr);
    srcRdmaAppQP->m_qp->setAppQp(srcRdmaAppQP);
    srcRdmaAppQP->m_qp->setRdmaHw(srcConnection->m_appQP->m_rdmaDriver->m_rdma);
    dstRdmaAppQP->m_qp->setAppQp(dstRdmaAppQP);
    dstRdmaAppQP->m_qp->setRdmaHw(dstConnection->m_appQP->m_rdmaDriver->m_rdma);
};

void DistributedStorageDaemon::StartApplication(void) {
    NS_LOG_FUNCTION_NOARGS();
    for(auto threads:m_threads){
        threads->Start();
    }
}

void DistributedStorageDaemon::StopApplication() {
    NS_LOG_FUNCTION_NOARGS();
    // TODO stop the queue pair
}


void DistributedStorageDaemon::DoDispose(void) {};

}  // Namespace ns3
