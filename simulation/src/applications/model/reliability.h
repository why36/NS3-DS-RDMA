/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c)
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
 * Author: Yixiao(Krayecho) Gao <532820040@qq.com>
 *
 */

#ifndef RELIABILITY_H
#define RELIABILITY_H

#include <bitset>
#include <map>
#include <queue>
#include <unordered_map>

#include "ns3/distributed-storage-daemon.h"
#include "ns3/pointer.h"
#include "ns3/rdma-queue-pair.h"
#include "ns3/uinteger.h"
#include "ns3/vector.h"

namespace ns3 {

static const int CHUNK_BIT = 9;
using RPCNumber = uint32_t;
using ACKChunk = struct ack_chunk {
    ack_chunk(uint32_t imm) : rpc_id(imm >> CHUNK_BIT), chunk_id(rpc_id & (((uint32_t)2 << (CHUNK_BIT + 1)) - 1)){};
    static uint32_t GetImm(uint32_t rpc_id, uint32_t chunk_id) { return (rpc_id << CHUNK_BIT) + chunk_id; };
    uint32_t GetImm() { return GetImm(rpc_id, chunk_id); };
    RPCNumber rpc_id;
    uint32_t chunk_id;
};

class ACK {
   public:
    std::vector<ACKChunk> chunks;
};

// class UserSpaceConnection;

class Reliability : public Object {
   public:
    uint32_t GetMessageNumber() { return m_messageNumber++; }
    uint32_t GetMessageTotalNumber() { return m_messageNumber; }
    uint64_t GetWRid() { return m_wruuid++; }

    void InsertWWR(Ptr<IBVWorkRequest> wr);
    void AckWR(uint32_t imm, uint64_t number);

    void SetUSC(Ptr<UserSpaceConnection> usc) { m_usc = usc; };
    // When an RPC is sent, the ChunkId of the RPC sent at this time is recorded
    std::map<uint32_t, uint16_t> tx_rpc_chunk;
    // key is rpc_id, value is the total chunk numer of this rpc
    std::map<uint32_t, uint16_t> rx_rpc_totalChunk;
    // Save RPC's verb for repass
    std::queue<Ptr<IBVWorkRequest>> rpcImm_verb;

   private:
    uint32_t m_messageNumber = 0;
    uint64_t m_wruuid = 0;
    Ptr<UserSpaceConnection> m_usc;
};

static const int MAX_CHUNK = 2 << CHUNK_BIT;
class RpcAckBitMap : public Object {
   public:
    RpcAckBitMap() {}
    ~RpcAckBitMap() {}
    void Set(uint32_t message_id, uint32_t chunk_id) {
        NS_ASSERT(chunk_id < 512);
        if (!m_maps.count(message_id)) {
            std::bitset<MAX_CHUNK> tmp_bitset;
            m_maps[message_id] = tmp_bitset;
        }
        m_maps[message_id].set(chunk_id);
    }

    bool Get(uint32_t message_id, uint32_t chunk_id) {
        NS_ASSERT(chunk_id < 512);
        if (!m_maps.count(message_id)) return false;
        return m_maps[message_id][chunk_id];
    }

    bool Check(uint32_t message_id, uint32_t max_chunk_id) {
        if (!m_maps.count(message_id)) return false;
        auto p_bitset = m_maps[message_id];
        for (int i = 0; i <= max_chunk_id; i++) {
            if (!p_bitset[i]) {
                return false;
            }
        }
        return true;
    }

    void Erase(uint32_t message_id) {
        if (!m_maps.count(message_id)) return;
        m_iter = m_maps.find(message_id);
        // delete m_maps[message_id];
        m_maps.erase(m_iter);
    }

   private:
    std::unordered_map<uint32_t, std::bitset<MAX_CHUNK>> m_maps;
    std::unordered_map<uint32_t, std::bitset<MAX_CHUNK>>::iterator m_iter;
};

inline void Reliability::InsertWWR(Ptr<IBVWorkRequest> wr) { rpcImm_verb.push(wr); };
inline void Reliability::AckWR(uint32_t imm, uint64_t wr_id) {
    NS_ASSERT(wr_id >= rpcImm_verb.front()->wr_id);
    while (rpcImm_verb.front()->wr_id <= wr_id) {
        if (rpcImm_verb.front()->wr_id < wr_id) {
            auto wr = rpcImm_verb.front();
            m_usc->Retransmit(wr);
        } else {
            m_usc->ReceiveAck(rpcImm_verb.front());
            rpcImm_verb.pop();
            return;
        }
    }
};

}  // namespace ns3
#endif /* RELIABILITY_H */
