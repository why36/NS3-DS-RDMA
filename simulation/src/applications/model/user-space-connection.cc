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
#include "ns3/rpc.h"
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
    NS_LOG_FUNCTION(this);
    m_userspace_congestion_control = CreateObject<LeapCC>();
    m_chunking = CreateObject<LinearRTTChunking>();
    m_reliability = Create<Reliability>();
    m_reliability->set_userspace_connection(Ptr<UserSpaceConnection>(this));
    m_remaining_size = 0;
}

void UserSpaceConnection::SetReceiveRPCCallback(Callback<void, Ptr<RPC>> cb) {
    NS_LOG_FUNCTION(this);
    m_receiveRPCCB = cb;
};

void UserSpaceConnection::Retransmit(Ptr<IBVWorkRequest> wc) {
    NS_LOG_FUNCTION(this);
    m_retransmissions.push(wc);
    SendRetransmissions();
};

void UserSpaceConnection::SendRPC(Ptr<RPC> rpc) {
    NS_LOG_FUNCTION(this);
    rpc->m_info.issue_time = Simulator::Now().GetMicroSeconds();
    m_queuing_rpcs.push(rpc);
    DoSend();
    StartDequeueAndTransmit();
}

void UserSpaceConnection::StartDequeueAndTransmit() {
    NS_LOG_FUNCTION(this);
    uint32_t nic_idx = m_app_qp->get_rdma_driver()->get_rdma_hw()->GetNicIdxOfQp(m_app_qp->m_qp);
    Ptr<QbbNetDevice> dev = m_app_qp->get_rdma_driver()->get_rdma_hw()->m_nic[nic_idx].dev;
    dev->TriggerTransmit();
}

void UserSpaceConnection::DoSend() {
    NS_LOG_FUNCTION(this);
    SendRetransmissions();
    SendNewRPC();
}

void UserSpaceConnection::SendRetransmissions() {
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_userspace_congestion_control->GetCongestionContorlType() == RTTWindowCCType);
    Ptr<RttWindowCongestionControl> cc_implement =
        DynamicCast<RttWindowCongestionControl, UserSpaceCongestionControl>(m_userspace_congestion_control);
    uint32_t cc_size = cc_implement->GetCongestionWindow();
    while (cc_size && !m_retransmissions.empty()) {
        auto wr = m_retransmissions.front();
        m_retransmissions.pop();

        wr->wr_id = m_reliability->GetWRid();
        Ptr<WRidTag> wr_id_tag1 = Create<WRidTag>();
        wr_id_tag1->SetWRid(wr->wr_id);
        wr->tags.wr_id_tag = wr_id_tag1;
        m_reliability->InsertWR(wr);
        m_app_qp->PostSend(wr);

        cc_implement->IncreaseInflight(wr->size);
        cc_size = cc_implement->GetCongestionWindow();
    }
};

void UserSpaceConnection::SendNewRPC() {
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_userspace_congestion_control->GetCongestionContorlType() == RTTWindowCCType);
    Ptr<RttWindowCongestionControl> cc_implement =
        DynamicCast<RttWindowCongestionControl, UserSpaceCongestionControl>(m_userspace_congestion_control);
    uint32_t cc_size = cc_implement->GetCongestionWindow();

    while (cc_size) {
        if (m_remaining_size) {
            uint32_t chunk_size = m_chunking->GetChunkSize(cc_size);
            Ptr<IBVWorkRequest> wr = Create<IBVWorkRequest>();
            if (m_remaining_size > chunk_size) {
                wr->size = chunk_size;
            } else if (m_remaining_size <= chunk_size) {
                wr->size = m_remaining_size;
            }
            uint32_t chunk_id = m_reliability->GetNewChunkId(m_sending_rpc->m_usc_id);
            NS_LOG_LOGIC("sending wr with rpc_id: " << (int)m_sending_rpc->rpc_id << " usc_id: " << (int)m_sending_rpc->m_usc_id << " chunk_id "
                                                    << (int)chunk_id << std::endl);

            wr->imm = ACKChunk::GetImm(m_sending_rpc->m_usc_id, chunk_id);

            Ptr<WRidTag> wr_id_tag = Create<WRidTag>();
            wr->wr_id = m_reliability->GetWRid();
            wr_id_tag->SetWRid(wr->wr_id);
            Ptr<ChunkSizeTag> chunkSizeTag = Create<ChunkSizeTag>();
            chunkSizeTag->SetChunkSize(chunk_size);
            Ptr<RPCTag> rpcTag = Create<RPCTag>();
            rpcTag->SetRPCId(m_sending_rpc->rpc_id);
            rpcTag->SetRequestSize(m_sending_rpc->m_request_size);
            rpcTag->SetResponseSize(m_sending_rpc->m_response_size);
            rpcTag->SetRPCUSCId(m_sending_rpc->m_usc_id);
            rpcTag->SetRPCReqResType(m_sending_rpc->m_rpc_type);
            wr->tags.wr_id_tag = wr_id_tag;
            wr->tags.chunk_size_tag = chunkSizeTag;
            wr->tags.rpc_tag = rpcTag;

            if (m_remaining_size == wr->size) {
                Ptr<RPCTotalOffsetTag> rpcTotalOffsetTag = Create<RPCTotalOffsetTag>();
                rpcTotalOffsetTag->SetRPCTotalOffset(chunk_id);
                wr->tags.rpctotaloffset_tag = rpcTotalOffsetTag;
                wr->tags.mark_tag_bits = kLastTagPayloadBits;
                NS_LOG_LOGIC("sending packet with last tag payload");
            } else {
                wr->tags.mark_tag_bits = kGeneralTagPayloadBits;
                NS_LOG_LOGIC("sending packet without last tag payload");
            }
            wr->verb = IBVerb::IBV_SEND_WITH_IMM;
            m_reliability->InsertWR(wr);
            m_app_qp->PostSend(wr);
            m_remaining_size = m_remaining_size > wr->size ? m_remaining_size - wr->size : 0;

            cc_implement->IncreaseInflight(wr->size);
            cc_size = cc_implement->GetCongestionWindow();
        } else {
            if (m_sending_rpc) {
                m_reliability->DeleteChunkIds(m_sending_rpc->m_usc_id);
            }

            if (m_queuing_rpcs.empty()) {
                m_sending_rpc = nullptr;
                break;
            } else if (!m_queuing_rpcs.empty()) {
                if (!m_remaining_size) {
                    m_sending_rpc = m_queuing_rpcs.front();
                    m_queuing_rpcs.pop();
                    m_sending_rpc->m_usc_id = m_reliability->GetMessageNumber();
                    m_remaining_size =
                        (m_sending_rpc->m_rpc_type == RPCType::Request) ? m_sending_rpc->m_request_size : m_sending_rpc->m_response_size;
                }
            }
        }
    }
};

void UserSpaceConnection::SendAck(uint32_t _imm, Ptr<WRidTag> wrid_tag) {
    NS_LOG_FUNCTION(this);
    Ptr<IBVWorkRequest> m_sendAckWr = Create<IBVWorkRequest>();  // There's only one slice

    Ptr<ChunkSizeTag> chunkSizeTag = Create<ChunkSizeTag>();
    chunkSizeTag->SetChunkSize(0);
    Ptr<RPCTag> rpcTag = Create<RPCTag>();
    rpcTag->SetRPCReqResType(RPCType::Message);

    rpcTag->SetRPCUSCId(0);
    Ptr<RPCTotalOffsetTag> rpcTotalOffsetTag;
    rpcTotalOffsetTag->SetRPCTotalOffset(0);
    m_sendAckWr->imm = _imm;
    m_sendAckWr->tags.wr_id_tag = wrid_tag;
    m_sendAckWr->tags.chunk_size_tag = chunkSizeTag;
    m_sendAckWr->tags.rpc_tag = rpcTag;
    m_sendAckWr->tags.rpctotaloffset_tag = rpcTotalOffsetTag;
    m_sendAckWr->tags.mark_tag_bits = kLastTagPayloadBits;
    m_sendAckWr->size = ACK_size;  // ack size
    m_sendAckWr->verb = IBVerb::IBV_SEND_WITH_IMM;
    m_ack_qp->PostSendAck(m_sendAckWr);
}

void UserSpaceConnection::ReceiveAck(Ptr<IBVWorkRequest> ackWR) {
    NS_LOG_FUNCTION(this);
    Ptr<RttWindowCongestionControl> cc_implement =
        DynamicCast<RttWindowCongestionControl, UserSpaceCongestionControl>(m_userspace_congestion_control);
    RTTSignal rtt;
    if (this->m_app_qp->GetQPType() == QPType::RDMA_RC) {
        rtt.m_rtt = Simulator::Now().GetMicroSeconds() - ackWR->send_completion_time;
    } else if (this->m_app_qp->GetQPType() == QPType::RDMA_UC) {
        // krayecho: simplified by immediate ack return
        rtt.m_rtt = Simulator::Now().GetMicroSeconds() - ackWR->send_completion_time;
    } else {
        NS_ASSERT(false);
    }

    cc_implement->UpdateSignal(&rtt);
    cc_implement->DecreaseInflight(ackWR->size);

    ChunkingSignal chunkSignal;
    chunkSignal.rtt = rtt.m_rtt;
    Ptr<LinearRTTChunking> chunksing = DynamicCast<LinearRTTChunking, ChunkingInterface>(m_chunking);
    m_chunking->UpdateChunkSize(chunkSignal);
}

void UserSpaceConnection::OnTxIBVWC(Ptr<IBVWorkCompletion> txIBVWC) {
    NS_LOG_FUNCTION(this);
    if (m_app_qp->GetQPType() == QPType::RDMA_RC) {
        m_reliability->AckWR(txIBVWC->imm, txIBVWC->wr->wr_id);
    }
    return;
}

void UserSpaceConnection::OnRxIBVWC(Ptr<IBVWorkCompletion> rxIBVWC) {
    NS_LOG_FUNCTION(this);
    if (m_app_qp->GetQPType() == QPType::RDMA_UC) {
        if (rxIBVWC->tags.rpc_tag->GetRPCReqResType() == RPCType::Message) {
            m_reliability->AckWR(rxIBVWC->imm, rxIBVWC->tags.wr_id_tag->GetWRid());
        } else {
            SendAck(rxIBVWC->imm, rxIBVWC->tags.wr_id_tag);
            ACKChunk chunk(rxIBVWC->imm);
            uint32_t usc_id = chunk.usc_id;
            uint32_t chunk_id = chunk.chunk_id;
            m_rpc_ack_bitmap->Set(usc_id, chunk_id);

            NS_ASSERT(rxIBVWC->tags.mark_tag_bits & RPCTOTALOFFSET_BIT);
            m_reliability->SetTotalChunks(usc_id, rxIBVWC->tags.rpctotaloffset_tag->GetRPCTotalOffset());
            if (m_rpc_ack_bitmap->Check(usc_id, m_reliability->GetTotalChunks(usc_id))) {
                m_reliability->DeleteTotalChunks(usc_id);

                Ptr<RPC> rpc = Create<RPC>(rxIBVWC->tags.rpc_tag->GetRPCId(), rxIBVWC->tags.rpc_tag->GetRequestSize(),
                                           rxIBVWC->tags.rpc_tag->GetResponseSize(), rxIBVWC->tags.rpc_tag->GetRPCReqResType());
                m_receiveRPCCB(rpc);
            }
        }
    } else if (m_app_qp->GetQPType() == QPType::RDMA_RC) {
        NS_ASSERT(rxIBVWC->tags.rpc_tag->GetRPCReqResType() != RPCType::Message);
        if (rxIBVWC->tags.mark_tag_bits & RPCTOTALOFFSET_BIT) {
            NS_LOG_LOGIC("usc receives a last wc");
            ACKChunk chunk(rxIBVWC->imm);
            // std::cout << " the chunk is: usc_id " << std::dec << chunk.usc_id << "chunk_id " << chunk.chunk_id << "\n the total offset is "
            //<< (rxIBVWC->tags.rpctotaloffset_tag->GetRPCTotalOffset());

            if ((static_cast<uint16_t>(chunk.chunk_id)) ==
                rxIBVWC->tags.rpctotaloffset_tag->GetRPCTotalOffset()) {  // the ibv_wr has been collected Completely
                Ptr<RPC> rpc = Create<RPC>(rxIBVWC->tags.rpc_tag->GetRPCId(), rxIBVWC->tags.rpc_tag->GetRequestSize(),
                                           rxIBVWC->tags.rpc_tag->GetResponseSize(), rxIBVWC->tags.rpc_tag->GetRPCReqResType());
                m_receiveRPCCB(rpc);
            }
        } else {
            NS_LOG_LOGIC("usc receives a middle wc");
        }
    }
}

Ptr<RdmaAppQP> UserSpaceConnection::get_app_qp() { return m_app_qp; }
void UserSpaceConnection::set_app_qp(Ptr<RdmaAppQP> app_qp) { m_app_qp = app_qp; }

}  // Namespace ns3
