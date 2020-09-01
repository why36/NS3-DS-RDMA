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

UserSpaceConnection::UserSpaceConnection(){
    m_appQP->setUSC(this);
}

void UserSpaceConnection::SendRPC(Ptr<RPC> rpc) {
    m_sendQueuingRPCs.push(rpc);
    SendRPC();
}

void UserSpaceConnection::SendRPC() {
    if (m_remainingSendingSize) {
        uint32_t flowsegSize = m_flowseg->GetSegSize();
        Ptr<IBVWorkRequest> wr;
        if (m_remainingSendingSize > flowsegSize) {
            wr = Create<IBVWorkRequest>();
            wr->size = flowsegSize;
        } else if (m_remainingSendingSize <= flowsegSize) {
            wr = Create<IBVWorkRequest>(4);
            wr->size = m_remainingSendingSize;
        }
        wr->imm = (m_sendingRPC->rpc_id) << 9 + m_reliability->rpc_seg[m_sendingRPC->rpc_id];
        // tags assignment
        Ptr<FlowSegSizeTag> flowSegSizeTag = Create<FlowSegSizeTag>();
        flowSegSizeTag->SetFlowSegSize(flowsegSize);
        Ptr<RPCSizeTag> rpcSizeTag = Create<RPCSizeTag>();
        rpcSizeTag->SetRPCSize(m_sendingRPC->m_rpc_size);
        Ptr<RPCRequestResponseTypeIdTag>  RPCReqResTag= Create<RPCRequestResponseTypeIdTag>();
        RPCReqResTag.SetRPCReqResId(m_sendingRPC->m_reqres_id);
        RPCReqResTag.SetRPCReqResType(m_sendingRPC->m_rpc_type);
        wr->tags[0] = flowSegSizeTag;
        wr->tags[1] = rpcSizeTag;
        wr->tags[2] = RPCReqResTag;
        if (m_remainingSendingSize == wr->size) {
            Ptr<RPCTotalOffsetTag> rpcTotalOffsetTag;
            rpcTotalOffsetTag->SetRPCTotalOffset(m_reliability->rpc_seg[m_sendingRPC->rpc_id]);
            wr->tags[3] = rpcTotalOffsetTag;
            m_reliability->rpc_totalSeg.insert(std::pair<uint32_t, uint16_t>(m_sendingRPC->rpc_id, m_reliability->rpc_seg[m_sendingRPC->rpc_id]));
        }else{
            m_reliability->rpc_seg[m_sendingRPC->rpc_id]++;
        }
        wr->verb = IBVerb::IBV_SEND_WITH_IMM;
        m_appQP->PostSend(wr);
        m_reliability->rpcImm_verb.insert(std::pair<uint32_t, Ptr<IBVWorkRequest>>(wr->imm, wr));
        m_remainingSendingSize = m_remainingSendingSize > wr->size ? m_remainingSendingSize - wr->size : 0;
    }
    if (m_remainingSendingSize == 0) {
        if (!m_sendQueuingRPCs.empty()) {
            m_sendingRPC = m_sendQueuingRPCs.front();
            m_sendQueuingRPCs.pop();
            m_sendingRPC->rpc_id = m_reliability->GetMessageNumber();
            m_reliability->rpc_seg.insert(std::pair<uint32_t, uint16_t>(m_sendingRPC->rpc_id, 0));
            m_remainingSendingSize = m_sendingRPC->m_rpc_size;
        } else {
            m_sendingRPC = nullptr;
        }
    }
}

void UserSpaceConnection::SendAck(uint32_t _imm) {
    Ptr<IBVWorkRequest> m_sendAckWr = Create<IBVWorkRequest>(4); //There's only one slice
    m_sendAckWr->imm = _imm;
    m_sendQueuingAckWr.push(m_sendAckWr);
    SendAck();
}

void UserSpaceConnection::SendAck() {
    Ptr<IBVWorkRequest> m_sendingAckWr;
    if (!m_sendQueuingAckWr.empty()) {
        m_sendingAckWr = m_sendQueuingAckWr.front();
        m_sendQueuingAckWr.pop();
    } else {
        m_sendingAckWr = nullptr;
        return;
    }
    Ptr<FlowSegSizeTag> flowSegSizeTag = Create<FlowSegSizeTag>();
    flowSegSizeTag->SetFlowSegSize(0);
    Ptr<RPCSizeTag> rpcSizeTag = Create<RPCSizeTag>();
    rpcSizeTag->SetRPCSize(100);
    Ptr<RPCTotalOffsetTag> rpcTotalOffsetTag;
    rpcTotalOffsetTag->SetRPCTotalOffset(0);
    Ptr<RPCRequestResponseTypeIdTag>  RPCReqResTag= Create<RPCRequestResponseTypeIdTag>();
    RPCReqResTag.SetRPCReqResId(0);
    RPCReqResTag.SetRPCReqResType(0);
    m_sendingAckWr->tags[0] = flowSegSizeTag;
    m_sendingAckWr->tags[1] = rpcSizeTag;
    m_sendingAckWr->tags[2] = RPCReqResTag;
    m_sendingAckWr->tags[3] = rpcTotalOffsetTag;
    m_sendingAckWr->size = 100;//ack size
    m_ackQP->PostSendAck(m_sendingAckWr);
}

void UserSpaceConnection::ReceiveAck(Ptr<IBVWorkCompletion> m_ackWc) {
    m_receiveQueuingAckWc.push(m_ackWc);
    ReceiveAck();
}

void UserSpaceConnection::ReceiveAck() {
    Ptr<IBVWorkCompletion> m_receivingAckWr;
    if (!m_receiveQueuingAckWc.empty()) {
        m_receivingAckWr = m_receiveQueuingAckWc.front();
        m_receiveQueuingAckWc.pop();
    } else {
        m_receivingAckWr = nullptr;
        return;
    }
    // repass
    uint32_t ack_imm = m_receivingAckWr->imm;

    // If the verb has already been sent
    if (m_reliability->rpcImm_verb.find(ack_imm) != m_reliability->rpcImm_verb.end()) {
        m_appQP->PostSend(m_reliability->rpcImm_verb[ack_imm]);
    } else {
        // nothing to do
    }
}

void UserSpaceConnection::ReceiveIBVWC(Ptr<IBVWorkCompletion> receivingIBVWC) {
    m_receiveQueuingIBVWCs.push(receivingIBVWC);
    ReceiveIBVWC();
}

void UserSpaceConnection::ReceiveIBVWC() {
    if (!m_receiveQueuingIBVWCs.empty()) {
        m_receivingIBVWC = m_receiveQueuingIBVWCs.front();
        m_receiveQueuingIBVWCs.pop();
    } else {
        m_receivingIBVWC = nullptr;
        return;
    }
    if (!m_rpcAckBitMap.get(m_receivingIBVWC->imm)) m_rpcAckBitMap.set(m_receivingIBVWC->imm);
    m_receive_ibv_num++;
    // receive enough ibvwc, generate ack
    if (m_receive_ibv_num > m_ackQP->m_milestone_rx + m_ackQP->m_ack_qp_interval) {
        for (uint32_t i = 0; i < m_reliability->GetMessageTotalNumber(); i++) {
            if (m_reliability->rpc_finish.find(i) == m_reliability->rpc_finish.end()) {
                for (uint32_t j = 0; j < m_reliability->rpc_totalSeg[i]; j++) {
                    if (!m_rpcAckBitMap.get(i << 9 + j & 0x1FF)) {
                        SendAck(i << 9 + j & 0x1FF);  // next value want to be set
                        break;
                    }
                    if (j == m_reliability->rpc_totalSeg[i]) {
                        m_reliability->rpc_finish.insert(std::pair<uint32_t, bool>(i, true));
                        //if it identifies as a request, then reply with a Response ,also SendRPC(rpc);
                    }
                }
            }
        }
        m_ackQP->m_milestone_rx += m_ackQP->m_ack_qp_interval;
    }
}

TypeId DistributedStorageClient::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::DistributedStorageClient").SetParent<RdmaClient>().AddConstructor<DistributedStorageClient>();
    return tid;
}

DistributedStorageClient::DistributedStorageClient() { NS_LOG_FUNCTION_NOARGS(); }

DistributedStorageClient::~DistributedStorageClient() { NS_LOG_FUNCTION_NOARGS(); }

void DistributedStorageClient::SendRpc(Ptr<RPC> rpc) {
    // find user-space connection
    // send rpc

    Ptr<UserSpaceConnection> connection = Create<UserSpaceConnection>();
    connection->SendRPC(rpc);
};

void DistributedStorageClient::Connect(Ptr<DistributedStorageClient> client, Ptr<DistributedStorageClient> server, uint16_t pg, uint32_t size) {
    uint16_t sport = client->GetNextAvailablePort();
    uint16_t dport = server->GetNextAvailablePort();
    NS_ASSERT(sport && dport);

    Ptr<Node> client_node = client->GetNode();
    // Ptr<RdmaAppQP> srcRdmaAppQP(client_node->GetObject<RdmaDriver>(), DistributedStorageClient::OnResponse,
    //                            DistributedStorageClient::OnSendCompletion, DistributedStorageClient::OnReceiveCompletion);

    Ptr<UserSpaceConnection> srcConnection = Create<UserSpaceConnection>();
    Ptr<RdmaAppQP> srcRdmaAppQP =
        Create<RdmaAppQP>(client_node->GetObject<RdmaDriver>(), MakeCallback(&DistributedStorageClient::OnResponse, GetPointer(client)),
                          MakeCallback(&DistributedStorageClient::OnSendCompletion, GetPointer(client)),
                          MakeCallback(&DistributedStorageClient::OnReceiveCompletion, GetPointer(client)));
    srcConnection->appQp = srcRdmaAppQP;
    client->m_Connections.push_back(srcConnection);

    //client->AddQP(srcRdmaAppQP);

    Ptr<Node> server_node = server->GetNode();
    // Ptr<RdmaAppQP> dstRdmaAppQP(server_node->GetObject<RdmaDriver>(), DistributedStorageClient::OnResponse,
    //                            DistributedStorageClient::OnSendCompletion, DistributedStorageClient::OnReceiveCompletion);

    Ptr<UserSpaceConnection> dstConnection = Create<UserSpaceConnection>();
    Ptr<RdmaAppQP> dstRdmaAppQP =
        Create<RdmaAppQP>(server_node->GetObject<RdmaDriver>(), MakeCallback(&DistributedStorageClient::OnResponse, GetPointer(server)),
                          MakeCallback(&DistributedStorageClient::OnSendCompletion, GetPointer(server)),
                          MakeCallback(&DistributedStorageClient::OnReceiveCompletion, GetPointer(server)));
    dstConnection->appQp = dstRdmaAppQP;
    server->m_Connections.push_back(srcConnection);
    //server->AddQP(dstRdmaAppQP);

    QpParam srcParam(client->GetSize(), client->m_win, client->m_baseRtt, MakeCallback(&DistributedStorageClient::Finish, client));
    QpParam dstParam(server->GetSize(), server->m_win, server->m_baseRtt, MakeCallback(&DistributedStorageClient::Finish, server));
    QPConnectionAttr srcConnAttr(pg, client->m_ip, server->m_ip, sport, dport, QPType::RDMA_RC);
    RdmaCM::Connect(srcRdmaAppQP, dstRdmaAppQP, srcConnAttr, srcParam, dstParam);
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

void DistributedStorageClient::OnResponse(Ptr<RpcResponse> rpcResponse, Ptr<RdmaQueuePair> qp){

}

void DistributedStorageClient::DoDispose(void){

}

void DistributedStorageClient::OnSendCompletion(Ptr<IBVWorkCompletion> completion){

}

void DistributedStorageClient::OnReceiveCompletion(Ptr<IBVWorkCompletion> completion){

}

}  // Namespace ns3
