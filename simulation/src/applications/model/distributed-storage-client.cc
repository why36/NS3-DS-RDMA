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

#include "ns3/distributed-storage-client.h"

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
#include "ns3/rpc-request.h"
#include "ns3/rpc-response.h"
#include "ns3/seq-ts-header.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "ns3/verb-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DistributedStorageClient");
NS_OBJECT_ENSURE_REGISTERED(DistributedStorageClient);

TypeId DistributedStorageClient::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::DistributedStorageClient").SetParent<RdmaClient>().AddConstructor<DistributedStorageClient>();
    return tid;
}

DistributedStorageClient::DistributedStorageClient() { NS_LOG_FUNCTION_NOARGS(); }

DistributedStorageClient::~DistributedStorageClient() { NS_LOG_FUNCTION_NOARGS(); }

void DistributedStorageClient::Connect(Ptr<DistributedStorageClient> client, Ptr<DistributedStorageClient> server, uint16_t pg) {
    uint16_t sport = client->GetNextAvailablePort();
    uint16_t dport = server->GetNextAvailablePort();
    NS_ASSERT(sport && dport);

    Ptr<Node> client_node = client->GetNode();
    // Ptr<RdmaAppQP> srcRdmaAppQP(client_node->GetObject<RdmaDriver>(), DistributedStorageClient::OnResponse,
    //                            DistributedStorageClient::OnSendCompletion, DistributedStorageClient::OnReceiveCompletion);

    Ptr<RdmaAppQP> srcRdmaAppQP =
        Create<RdmaAppQP>(client_node->GetObject<RdmaDriver>(), MakeCallback(&DistributedStorageClient::OnResponse, GetPointer(client)),
                          MakeCallback(&DistributedStorageClient::OnSendCompletion, GetPointer(client)),
                          MakeCallback(&DistributedStorageClient::OnReceiveCompletion, GetPointer(client)));
    Ptr<UserSpaceConnection> srcConnection = Create<UserSpaceConnection>();
    // Bind with each other
    srcConnection->m_appQP = srcRdmaAppQP;
    srcRdmaAppQP->setUSC(srcConnection);

    client->m_Connections.push_back(srcConnection);

    // client->AddQP(srcRdmaAppQP);

    Ptr<Node> server_node = server->GetNode();
    // Ptr<RdmaAppQP> dstRdmaAppQP(server_node->GetObject<RdmaDriver>(), DistributedStorageClient::OnResponse,
    //                            DistributedStorageClient::OnSendCompletion, DistributedStorageClient::OnReceiveCompletion);

    Ptr<UserSpaceConnection> dstConnection = Create<UserSpaceConnection>();
    Ptr<RdmaAppQP> dstRdmaAppQP =
        Create<RdmaAppQP>(server_node->GetObject<RdmaDriver>(), MakeCallback(&DistributedStorageClient::OnResponse, GetPointer(server)),
                          MakeCallback(&DistributedStorageClient::OnSendCompletion, GetPointer(server)),
                          MakeCallback(&DistributedStorageClient::OnReceiveCompletion, GetPointer(server)));
    // Bind with each other
    dstConnection->m_appQP = dstRdmaAppQP;
    dstRdmaAppQP->setUSC(dstConnection);

    server->m_Connections.push_back(dstConnection);
    // server->AddQP(dstRdmaAppQP);

    QpParam srcParam(client->GetSize(), client->m_win, client->m_baseRtt, MakeCallback(&DistributedStorageClient::Finish, client));
    QpParam dstParam(server->GetSize(), server->m_win, server->m_baseRtt, MakeCallback(&DistributedStorageClient::Finish, server));
    QPConnectionAttr srcConnAttr(pg, client->m_ip, server->m_ip, sport, dport, QPType::RDMA_RC);
    RdmaCM::Connect(srcRdmaAppQP, dstRdmaAppQP, srcConnAttr, srcParam, dstParam);
    srcRdmaAppQP->m_qp->setAppQp(srcRdmaAppQP);
    srcRdmaAppQP->m_qp->setRdmaHw(srcConnection->m_appQP->m_rdmaDriver->m_rdma);
    dstRdmaAppQP->m_qp->setAppQp(dstRdmaAppQP);
    dstRdmaAppQP->m_qp->setRdmaHw(dstConnection->m_appQP->m_rdmaDriver->m_rdma);
};

void DistributedStorageClient::AddQP(Ptr<RdmaAppQP> qp){};

void DistributedStorageClient::StartApplication(void) {
    NS_LOG_FUNCTION_NOARGS();
    //
    // Get window size and  data Segment
    //
    // qp->ibv_post_send(segment_id, size);
    //
}

void DistributedStorageClient::StopApplication() {
    NS_LOG_FUNCTION_NOARGS();
    // TODO stop the queue pair
}

void DistributedStorageClient::OnResponse(Ptr<RpcResponse> rpcResponse, Ptr<RdmaQueuePair> qp) {}

void DistributedStorageClient::DoDispose(void) {}

// void DistributedStorageClient::OnSendCompletion(Ptr<IBVWorkCompletion> completion) { }

// void DistributedStorageClient::OnReceiveCompletion(Ptr<IBVWorkCompletion> completion) { }

}  // Namespace ns3
