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
#include "ns3/verb-tag.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("DistributedStorageClient");
    NS_OBJECT_ENSURE_REGISTERED(DistributedStorageClient);

    void UserSpaceConnection::SendRPC(Ptr<RPC> rpc) {
        m_sendQueuingRPCs.push_back(rpc);
        SendRPC();
    }

    void UserSpaceConnection::SendRPC() {
        if(m_remainingSendingSize)
        {
            uint32_t flowsegSize =  m_flowseg.GetSize();
            Ptr<IBVWorkRequest> wr;
            if(m_remainingSendingSize > flowsegSize)
            {
                wr = Create<IBVWorkRequest>();
                wr->size = flowsegSize;
            }
            else if(m_remainingSendingSize <= flowsegSize)
            {
                wr = Create<IBVWorkRequest>(3);
                wr->size = m_remainingSendingSize;
            }
            wr->imm = (m_sendingRPC->rpc_id) << 9 + m_reliability->rpc_seg[m_sendingRPC->rpc_id]++;
            Ptr<FlowSegSizeTag> flowSegSizeTag = Create<FlowSegSizeTag>();
            flowSegSizeTag->SetFlowSegSize(flowsegSize);
            Ptr<RPCSizeTag> rpcSizeTag = Create<RPCSizeTag>();
            rpcSizeTag->SetRPCSize(m_sendingRPC->size);
            wr->tags[0] = flowSegSizeTag;
            wr->tags[1] = rpcSizeTag;
            if(m_remainingSendingSize == wr->size)
            {
                Ptr<RPCTotalOffsetTag> rpcTotalOffsetTag;
                rpcTotalOffsetTag->SetRPCTotalOffset(m_sendingRPC->segment_id);
                wr->tags[2] = rpcTotalOffsetTag;
            }
            wr->verb = IBVerb::IBV_SEND_WITH_IMM;
            m_appQP->PostSend(wr);
            m_remainingSendingSize = m_remainingSendingSize > wr->size ? m_remainingSendingSize-wr->size : 0;      
        }  
        if(m_remainingSendingSize == 0)
        {
            if(!m_sendQueuingRPCs.empty())
            {
                m_sendingRPC = m_sendQueuingRPCs.front();
                m_sendQueuingRPCs.pop();
                m_sendingRPC->rpc_id = m_reliability.GetMessageNumber();
                m_reliability->rpc_seg.insert((pair<uint32_t,uint16_t>(m_sendingRPC->rpc_id,0)));
                m_remainingSendingSize = m_sendingRPC->size;
            }
            else
            {
                m_sendingRPC = nullptr;
            }
        }
    }

    void UserSpaceConnection::ReceiveIBVWC(Ptr<IBVWorkCompletion> receiveQueuingIBVWC){
        m_receiveQueuingIBVWCs.push_back(receiveQueuingIBVWC);
        ReceiveIBVWC();
    }

    void UserSpaceConnection::ReceiveIBVWC(){
        if(!receiveQueuingIBVWC.empty())
        {
            m_receivingIBVWC = m_receiveQueuingIBVWCs.front();
            m_receiveQueuingIBVWCs.pop();
        }
        else
        {
            m_receivingIBVWC = nullptr;
            return;
        }
        m_rpcAckBitMap.set(m_receivingIBVWC->imm);        
        //uint32_t rpc_id = m_receivingIBVWC->imm >> 9;
        //uint16_t segment_id =  m_receivingIBVWC->imm & 0x1FF;

        


    }


    TypeId DistributedStorageClient::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::DistributedStorageClient").SetParent<RdmaClient>().AddConstructor<DistributedStorageClient>();
        return tid;
    }

    DistributedStorageClient::DistributedStorageClient() {
        NS_LOG_FUNCTION_NOARGS();
    }

    DistributedStorageClient::~DistributedStorageClient() {
        NS_LOG_FUNCTION_NOARGS();
    }

    void DistributedStorageClient::SendRpc(Ptr<RPC> rpc) {

        // find user-space connection
        // send rpc

        Ptr<UserSpaceConnection> connection;
        connection->SendRpc(rpc);
    };


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

    void DistributedStorageClient::AddQP(Ptr<RdmaQueuePair> qp) {};

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
