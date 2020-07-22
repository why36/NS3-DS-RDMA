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
#include "ns3/seq-ts-header.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DistributedStorageClient");
NS_OBJECT_ENSURE_REGISTERED(DistributedStorageClient);

TypeId DistributedStorageClient::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::DistributedStorageClient").SetParent<RdmaClient>().AddConstructor<DistributedStorageClient>();
    return tid;
}

DistributedStorageClient::DistributedStorageClient() { NS_LOG_FUNCTION_NOARGS(); }

DistributedStorageClient::~DistributedStorageClient() { NS_LOG_FUNCTION_NOARGS(); }

static void DistributedStorageClient::Connect(Ptr<DistributedStorageClient> client, Ptr<DistributedStorageClient> server, uint16_t pg) {
    uint16_t sport = client->GetNextAvailablePort();
    uint16_t dport = server->GetNextAvailablePort();
    NS_ASSERT(sport && dport);

    Ptr<Node> client_node = client->GetNode();
    Ptr<RdmaAppQP> srcRdmaAppQP(client_node->GetObject<RdmaDriver>(), DistributedStorageClient::OnResponse,
                                DistributedStorageClient::OnSendCompletion, DistributedStorageClient::OnReceiveCompletion);
    client->AddQP(srcRdmaAppQP);

    Ptr<Node> server_node = server->GetNode();
    Ptr<RdmaAppQP> dstRdmaAppQP(server_node->GetObject<RdmaDriver>(), DistributedStorageClient::OnResponse,
                                DistributedStorageClient::OnSendCompletion, DistributedStorageClient::OnReceiveCompletion);
    server->AddQP(dstRdmaAppQP);

    QpParam srcParam(m_size, m_win, m_baseRtt, MakeCallback(&DistributedStorageClient::Finish, client));
    QpParam srcParam(m_size, m_win, m_baseRtt, MakeCallback(&DistributedStorageClient::Finish, server));
    QPConnectionAttr srcConnAttr(pg, client_node.m_ip, server_node.m_ip, sport, dport, QPType::RDMA_RC);
    RdmaCM::Connect(srcRdmaAppQP, dstRdmaAppQP, srcConnAttr, srcParam, dstParam);
};

void DistributedStorageClient::AddQP(Ptr<RdmaQueuePair> qp){};

void DistributedStorageClient::StartApplication(void) override {
    NS_LOG_FUNCTION_NOARGS();
    //
    // Get window size and  data Segment
    //
    // qp->ibv_post_send(segment_id, size);
    //
}

void DistributedStorageClient::StopApplication() override {
    NS_LOG_FUNCTION_NOARGS();
    // TODO stop the queue pair
}

}  // Namespace ns3
