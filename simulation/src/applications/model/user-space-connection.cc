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

#include "ns3/user-space-connection.h"

#include <ns3/rdma-driver.h>
#include <stdio.h>
#include <stdlib.h>

#include "ns3/assert.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-end-point.h"
#include "ns3/leapcc.h"
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

NS_LOG_COMPONENT_DEFINE("UserSpaceConnection");
NS_OBJECT_ENSURE_REGISTERED(UserSpaceConnection);

UserSpaceConnection::UserSpaceConnection() {
    m_UCC = Create<LeapCC>();
    m_chunking = Create<LinearRTTChunking>();
    m_reliability = Create<Reliability>();
    m_reliability->SetUSC(Ptr<UserSpaceConnection>(this));
    m_remainingSendingSize = 0;
    // m_appQP->setUSC(this);
}

void UserSpaceConnection::Retransmit(Ptr<IBVWorkRequest> wc) {
    m_retransmissions.push(wc);
    SendRetransmissions();
};

void UserSpaceConnection::SendRPC(Ptr<RPC> rpc) {
    // StartDequeueAndTransmit();
    rpc->m_info.issue_time = Simulator::Now().GetMicroSeconds();
    m_sendQueuingRPCs.push(rpc);
    // StartDequeueAndTransmit();
    // SendRPC();
    DoSend();
    StartDequeueAndTransmit();
}

void UserSpaceConnection::StartDequeueAndTransmit() {
    uint32_t nic_idx = m_appQP->m_rdmaDriver->m_rdma->GetNicIdxOfQp(m_appQP->m_qp);
    // uint32_t nic_idx = m_appQP->m_qp->m_rdma->GetNicIdxOfQp(m_appQP->m_qp);
    Ptr<QbbNetDevice> dev = m_appQP->m_rdmaDriver->m_rdma->m_nic[nic_idx].dev;
    dev->TriggerTransmit();
}

void UserSpaceConnection::DoSend() {
    SendRetransmissions();
    SendNewRPC();
}

void UserSpaceConnection::SendRetransmissions() {
    NS_ASSERT(m_UCC->GetCongestionContorlType() == RTTWindowCCType);
    Ptr<RttWindowCongestionControl> cc_implement = DynamicCast<RttWindowCongestionControl, UserSpaceCongestionControl>(m_UCC);
    uint32_t cc_size = cc_implement->GetCongestionWindow();
    while (cc_size && !m_retransmissions.empty()) {
        auto wr = m_retransmissions.front();
        m_retransmissions.pop();

        wr->wr_id = m_reliability->GetWRid();
        Ptr<WRidTag> wrid_tag1 = Create<WRidTag>();
        wrid_tag1->SetWRid(wr->wr_id);
        wr->tags.wrid_tag = wrid_tag1;

        m_appQP->PostSend(wr);
        m_reliability->InsertWWR(wr);

        cc_implement->IncreaseInflight(wr->size);
        cc_size = cc_implement->GetCongestionWindow();
    }
};

void UserSpaceConnection::SendNewRPC() {
    NS_ASSERT(m_UCC->GetCongestionContorlType() == RTTWindowCCType);
    Ptr<RttWindowCongestionControl> cc_implement = DynamicCast<RttWindowCongestionControl, UserSpaceCongestionControl>(m_UCC);
    uint32_t cc_size = cc_implement->GetCongestionWindow();

    while (cc_size) {
        if (m_remainingSendingSize) {
            uint32_t chunksize = m_chunking->GetChunkSize(cc_size);
            Ptr<IBVWorkRequest> wr;
            if (m_remainingSendingSize > chunksize) {
                wr = Create<IBVWorkRequest>();
                wr->size = chunksize;
            } else if (m_remainingSendingSize <= chunksize) {
                wr = Create<IBVWorkRequest>();
                wr->size = m_remainingSendingSize;
            }
            wr->imm = ACKChunk::GetImm(m_sendingRPC->rpc_id, m_reliability->tx_rpc_chunk[m_sendingRPC->rpc_id]);
            // tags assignment
            Ptr<WRidTag> wrid_tag = Create<WRidTag>();
            wr->wr_id = m_reliability->GetWRid();
            wrid_tag->SetWRid(wr->wr_id);

            Ptr<ChunkSizeTag> chunkSizeTag = Create<ChunkSizeTag>();
            chunkSizeTag->SetChunkSize(chunksize);
            Ptr<RPCSizeTag> rpcSizeTag = Create<RPCSizeTag>();
            rpcSizeTag->SetRPCSize(m_sendingRPC->m_rpc_size);
            Ptr<RPCRequestResponseTypeIdTag> RPCReqResTag = Create<RPCRequestResponseTypeIdTag>();
            RPCReqResTag->SetRPCReqResId(m_sendingRPC->m_reqres_id);
            RPCReqResTag->SetRPCReqResType(m_sendingRPC->m_rpc_type);
            wr->tags.wrid_tag = wrid_tag;
            wr->tags.chunksize_tag = chunkSizeTag;
            wr->tags.rpcsize_tag = rpcSizeTag;
            wr->tags.rpctype_tag = RPCReqResTag;

            if (m_remainingSendingSize == wr->size) {
                Ptr<RPCTotalOffsetTag> rpcTotalOffsetTag = Create<RPCTotalOffsetTag>();
                rpcTotalOffsetTag->SetRPCTotalOffset(m_reliability->tx_rpc_chunk[m_sendingRPC->rpc_id]);
                wr->tags.rpctotaloffset_tag = rpcTotalOffsetTag;
                wr->tags.mark_tag_bits = kLastTagPayloadBits;
                // m_reliability->rpc_totalChunk.insert(std::pair<uint32_t, uint16_t>(m_sendingRPC->rpc_id,
                // m_reliability->rpc_chunk[m_sendingRPC->rpc_id]));
            } else {
                m_reliability->tx_rpc_chunk[m_sendingRPC->rpc_id]++;
                wr->tags.mark_tag_bits = kGeneralTagPayloadBits;
            }
            wr->verb = IBVerb::IBV_SEND_WITH_IMM;
            m_appQP->PostSend(wr);
            m_reliability->InsertWWR(wr);
            m_remainingSendingSize = m_remainingSendingSize > wr->size ? m_remainingSendingSize - wr->size : 0;

            cc_implement->IncreaseInflight(wr->size);
            cc_size = cc_implement->GetCongestionWindow();
        }
        if (!m_remainingSendingSize) {
            if (m_sendQueuingRPCs.empty()) {
                m_sendingRPC = nullptr;
                break;
            } else if (!m_sendQueuingRPCs.empty()) {
                if (!m_remainingSendingSize) {
                    m_sendingRPC = m_sendQueuingRPCs.front();
                    m_sendQueuingRPCs.pop();
                    m_sendingRPC->rpc_id = m_reliability->GetMessageNumber();
                    m_reliability->tx_rpc_chunk.insert(std::pair<uint32_t, uint16_t>(m_sendingRPC->rpc_id, 0));
                    m_remainingSendingSize = m_sendingRPC->m_rpc_size;
                }
            }
        }
    }
};

void UserSpaceConnection::SendAck(uint32_t _imm, Ptr<WRidTag> wrid_tag) {
    Ptr<IBVWorkRequest> m_sendAckWr = Create<IBVWorkRequest>();  // There's only one slice

    Ptr<ChunkSizeTag> chunkSizeTag = Create<ChunkSizeTag>();
    chunkSizeTag->SetChunkSize(0);
    Ptr<RPCSizeTag> rpcSizeTag = Create<RPCSizeTag>();
    rpcSizeTag->SetRPCSize(ACK_size);
    Ptr<RPCTotalOffsetTag> rpcTotalOffsetTag;
    rpcTotalOffsetTag->SetRPCTotalOffset(0);
    Ptr<RPCRequestResponseTypeIdTag> RPCReqResTag = Create<RPCRequestResponseTypeIdTag>();
    RPCReqResTag->SetRPCReqResId(0);
    // RPCReqResTag->SetRPCReqResType(0);

    m_sendAckWr->tags.wrid_tag = wrid_tag;
    m_sendAckWr->tags.chunksize_tag = chunkSizeTag;
    m_sendAckWr->tags.rpcsize_tag = rpcSizeTag;
    m_sendAckWr->tags.rpctype_tag = RPCReqResTag;
    m_sendAckWr->tags.rpctotaloffset_tag = rpcTotalOffsetTag;
    m_sendAckWr->tags.mark_tag_bits = kLastTagPayloadBits;
    m_sendAckWr->size = ACK_size;  // ack size
    m_ackQP->PostSendAck(m_sendAckWr);
}

// To do.Krayecho Yx: fix ack
// To do.Krayecho Yx: break retransmission into small pieces
void UserSpaceConnection::ReceiveAck(Ptr<IBVWorkCompletion> ackWC) {
    // repass

    uint32_t ack_imm = ackWC->imm;

    Ptr<RttWindowCongestionControl> cc_implement = DynamicCast<RttWindowCongestionControl, UserSpaceCongestionControl>(m_UCC);
    RTTSignal rtt;
    rtt.mRTT = 10;
    cc_implement->UpdateSignal(&rtt);
    cc_implement->DecreaseInflight(ackWC->size);  // A size is missing

    ChunkingSignal chunkSignal;
    chunkSignal.rtt = 10;
    Ptr<LinearRTTChunking> chunksing = DynamicCast<LinearRTTChunking, ChunkingInterface>(m_chunking);
    m_chunking->UpdateChunkSize(chunkSignal);

    m_reliability->AckWR(ackWC->imm, ackWC->tags.wrid_tag->GetWRid());
}

void UserSpaceConnection::ReceiveIBVWC(Ptr<IBVWorkCompletion> receivingIBVWC) {
    if (m_appQP->m_qp->m_connectionAttr.qp_type == QPType::RDMA_UC) {
        SendAck(receivingIBVWC->imm, receivingIBVWC->tags.wrid_tag);

        ACKChunk chunk(receivingIBVWC->imm);
        m_rpcAckBitMap->Set(chunk.rpc_id, chunk.chunk_id);

        if (receivingIBVWC->tags.mark_tag_bits & RPCTOTALOFFSET) {
            m_reliability->rx_rpc_totalChunk[chunk.rpc_id] = receivingIBVWC->tags.rpctotaloffset_tag->GetRPCTotalOffset();
        }

        if (m_reliability->rx_rpc_totalChunk[chunk.rpc_id] && m_rpcAckBitMap->Check(chunk.rpc_id, m_reliability->rx_rpc_totalChunk[chunk.rpc_id])) {
            // if it identifies as a request, then reply with a Response ,also SendRPC(rpc);
            if (receivingIBVWC->tags.rpctype_tag->GetRPCReqResType() == RPCType::Request) {
                Ptr<RpcResponse> response = Create<RpcResponse>(128, receivingIBVWC->tags.rpctype_tag->GetRPCReqResId());
                SendRPC(response);
            } else if (receivingIBVWC->tags.rpctype_tag->GetRPCReqResType() == RPCType::Response) {
                KeepKRpc(receivingIBVWC->tags.rpctype_tag->GetRPCReqResId());
            }
        }
    } else if (m_appQP->m_qp->m_connectionAttr.qp_type == QPType::RDMA_RC) {
        // receive the last verbs
        if (receivingIBVWC->tags.mark_tag_bits & RPCTOTALOFFSET) {
            ACKChunk chunk(receivingIBVWC->imm);
            if ((static_cast<uint16_t>(chunk.chunk_id)) == receivingIBVWC->tags.rpctotaloffset_tag->GetRPCTotalOffset()) {
                if (receivingIBVWC->tags.rpctype_tag->GetRPCReqResType() == RPCType::Request) {
                    Ptr<RpcResponse> response = Create<RpcResponse>(128, receivingIBVWC->tags.rpctype_tag->GetRPCReqResId());
                    SendRPC(response);
                } else if (receivingIBVWC->tags.rpctype_tag->GetRPCReqResType() == RPCType::Response) {
                    KeepKRpc(receivingIBVWC->tags.rpctype_tag->GetRPCReqResId());
                }
            }
        }
    }
}

void UserSpaceConnection::init() {
    for (int i = 0; i < kRPCRequest; i++) {
        Ptr<RpcRequest> req = Create<RpcRequest>(requestSize, m_rpcid++);
        RPCRequestMap.insert(std::pair<uint64_t, Ptr<RPC>>(req->rpc_id, req));
    }
}

void UserSpaceConnection::SendKRpc() {
    for (it = RPCRequestMap.begin(); it != RPCRequestMap.end(); it++) {
        SendRPC(it->second);
        // device.DequeueAndTransmit();
    }
}

void UserSpaceConnection::KeepKRpc(uint64_t response_id) {
    // When response is received, it is removed from the Map
    it = RPCRequestMap.find(response_id);
    NS_ASSERT_MSG(it != RPCRequestMap.end(), "Received an invalid response.");
    if (it != RPCRequestMap.end()) {
        rpc_latency[record_index++] = Simulator::Now().GetMicroSeconds() - it->second->m_info.issue_time;
        if (record_index == kMaxRecordNumber) {
            for (int i = 0; i < kMaxRecordNumber; i++) {
                output << rpc_latency[i] << "\n";
            }
            record_index = 0;
        }
        RPCRequestMap.erase(it);
    }
    while (RPCRequestMap.size() <= kRPCRequest) {
        Ptr<RpcRequest> req = Create<RpcRequest>(requestSize, m_rpcid++);
        RPCRequestMap.insert(std::pair<uint64_t, Ptr<RPC>>(req->rpc_id, req));
        SendRPC(req);
    }
}

}  // Namespace ns3
