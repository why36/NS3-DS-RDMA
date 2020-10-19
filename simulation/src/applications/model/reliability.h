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
    ack_chunk(uint32_t imm) : rpc_id(imm >> CHUNK_BIT), chunk_id(imm & ((static_cast<uint32_t>(1) << CHUNK_BIT) - 1)){};
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
    Reliability();
    uint32_t GetMessageNumber() { return m_messageNumber++; }
    uint32_t GetMessageTotalNumber() { return m_messageNumber; }
    uint64_t GetWRid() { return m_wruuid++; }

    uint32_t GetNewChunkId(uint32_t rpc_id);
    void DeleteChunkIds(uint32_t rpc_id);
    uint32_t GetTotalChunks(uint32_t rpc_id);
    void SetTotalChunks(uint32_t rpc_id, uint32_t maximal_chunk);
    void DeleteTotalChunks(uint32_t rpc_id);
    void InsertWR(Ptr<IBVWorkRequest> wr);
    void AckWR(uint32_t imm, uint64_t number);

    void set_userspace_connection(Ptr<UserSpaceConnection> userspace_connection);
    // When an RPC is sent, the ChunkId of the RPC sent at this time is recorded
    std::map<uint32_t, uint16_t> m_tx_rpc_chunk;
    // key is rpc_id, value is the total chunk numer of this rpc
    std::map<uint32_t, uint16_t> m_rx_rpc_totalChunk;
    // Save RPC's verb for repass
    std::queue<Ptr<IBVWorkRequest>> m_rpc_verbs;

   private:
    uint32_t m_messageNumber;
    uint64_t m_wruuid;
    Ptr<UserSpaceConnection> m_userspace_connection;
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
        auto iter = m_maps.find(message_id);
        // delete m_maps[message_id];
        m_maps.erase(iter);
    }

   private:
    std::unordered_map<uint32_t, std::bitset<MAX_CHUNK>> m_maps;
};

inline Reliability::Reliability() : m_messageNumber(0), m_wruuid(0){};
inline uint32_t Reliability::GetNewChunkId(uint32_t rpc_id) {
    if (m_tx_rpc_chunk.count(rpc_id) == 0) {
        m_tx_rpc_chunk[rpc_id] = 0;
    }
    return m_tx_rpc_chunk[rpc_id]++;
};
inline void Reliability::DeleteChunkIds(uint32_t rpc_id) {
    NS_ASSERT(m_tx_rpc_chunk.count(rpc_id) == 1);
    m_tx_rpc_chunk.erase(rpc_id);
};
inline uint32_t Reliability::GetTotalChunks(uint32_t rpc_id) {
    NS_ASSERT(m_rx_rpc_totalChunk.count(rpc_id) == 1);
    return m_rx_rpc_totalChunk[rpc_id];
}
inline void Reliability::SetTotalChunks(uint32_t rpc_id, uint32_t maximal_chunk) { m_rx_rpc_totalChunk[rpc_id] = maximal_chunk; };
inline void Reliability::DeleteTotalChunks(uint32_t rpc_id) {
    NS_ASSERT(m_rx_rpc_totalChunk.count(rpc_id) == 1);
    m_rx_rpc_totalChunk.erase(rpc_id);
};
inline void Reliability::InsertWR(Ptr<IBVWorkRequest> wr) { m_rpc_verbs.push(wr); };
inline void Reliability::AckWR(uint32_t imm, uint64_t wr_id) {
    NS_ASSERT(wr_id >= m_rpc_verbs.front()->wr_id);
    while (m_rpc_verbs.front()->wr_id <= wr_id) {
        if (m_rpc_verbs.front()->wr_id < wr_id) {
            auto wr = m_rpc_verbs.front();
            m_userspace_connection->Retransmit(wr);
        } else {
            m_userspace_connection->ReceiveAck(m_rpc_verbs.front());
            m_rpc_verbs.pop();
            return;
        }
    }
};
inline void Reliability::set_userspace_connection(Ptr<UserSpaceConnection> userspace_connection) { m_userspace_connection = userspace_connection; }

}  // namespace ns3
#endif /* RELIABILITY_H */
