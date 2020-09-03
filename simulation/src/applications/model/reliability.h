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
#include <hash_map>
#include <map>

#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/vector.h"

namespace ns3 {

static const SEG_BIT = 9;
using RPCNumber = uint32_t;
using ACKSeg = struct ack_seg {
    ACKSeg(uint32_t imm) : rpc_id(imm >> SEG_BIT), segment_id(rpc_id & (((uint32_t)2 << (SEG_BIT + 1)) - 1)){};
    static uint32_t GetImm(uint32_t rpc_id, uint32_t segment_id) { return (rpc_id << SEG_BIT) + segment_id; };
    uint32_t GetImm() { return GetImm(rpc_id, segment_id); };
    RPCNumber rpc_id;
    uint32_t segment_id;
};

class ACK {
   public:
    std::vector<ACKSeg> segments;
};

class UserSpaceConnection;

class Reliability : Object {
   public:
    uint32_t GetMessageNumber() { return m_messageNumber++; }
    uint32_t GetMessageTotalNumber() { return m_messageNumber; }
    uint64_t GetWRid(){return m_wruuid++};

    void InsertWWR(Ptr<IBVWorkRequest> wr);
    void AckWR(uint32_t imm, uint64_t number);

    void SetUSC(Ptr<UserSpaceConnection> usc) { m_usc = usc; };
    // When an RPC is sent, the SegId of the RPC sent at this time is recorded
    std::map<uint32_t, uint16_t> tx_rpc_seg;
    // key is rpc_id, value is the total seg numer of this rpc
    std::map<uint32_t, uint16_t> rx_rpc_totalSeg;
    // Save RPC's verb for repass
    std::queue<Ptr<IBVWorkRequest>> rpcImm_verb;

   private:
    uint32_t m_messageNumber = 0;
    uint64_t m_wruuid = 0;
    Ptr<UserSpaceConnection> m_usc;
};

static const MAX_SEG = 2 << SEG_BIT;
class RpcAckBitMap {
   public:
    RpcAckBitMap() {}
    ~RpcAckBitMap() {}
    void Set(uint32_t message_id, uint32_t segment_id) {
        NS_ASSERT(segment_id < 512);
        if (!m_maps.count(message_id)) {
            m_maps[message_id] = new std::bitset<MAX_SEG>;
        }
        m_maps[message_id]->Set(segment_id);
    }

    bool Get(uint32_t message_id, uint32_t segment_id) {
        NS_ASSERT(segment_id < 512);
        if (!m_maps.count(message_id)) return false;
        return m_maps[message_id]->Get(segment_id);
    }

    bool Check(uint32_t message_id, uint32_t max_segment_id) {
        if (!m_maps.count(message_id)) return false;
        auto p_bitset = m_maps[message_id];
        for (int i = 0; i <= max_segment_id; i++) {
            if (!p_bitset->Get(i)) {
                return false;
            }
        }
        return true;
    }

    void Erase(uint32_t message_id) {
        if (!m_maps.count(message_id)) return false;
        delete m_maps[message_id];
    }

   private:
    std::hash_map<uint32_t, std::bitset<MAX_SEG> * bitmap> m_maps;
};

}  // namespace ns3
#endif /* RELIABILITY_H */
