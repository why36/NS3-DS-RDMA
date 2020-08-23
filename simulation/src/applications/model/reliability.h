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

#include <map>

#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/vector.h"

namespace ns3 {

using RPCNumber = uint32_t;

using ACKSeg = struct ack_seg {
    RPCNumber rpc_id;
    uint16_t segment_id;
};

class ACK {
   public:
    std::vector<ACKSeg> segments;
};

class Reliability {
   public:
    uint32_t GetMessageNumber() { return m_messageNumber++; }
    uint32_t GetMessageTotalNumber() { return m_messageNumber; }
    // When an RPC is sent, the SegId of the RPC sent at this time is recorded
    std::map<uint32_t, uint16_t> rpc_seg;
    //key is rpc_id, value is the total seg numer of this rpc
    std::map<uint32_t,uint16_t> rpc_totalSeg;
    //indicates whether the RPC is complete
    std::map<uint32_t,bool> rpc_finish;
    //Save RPC's verb for repass
    std::map<uint32_t,Ptr<IBVWorkRequest>> rpcImm_verb;
   private:
    uint32_t m_messageNumber = 0;
};

class RpcAckBitMap {
   public:
    RpcAckBitMap() {
        this->arr = new uint8_t[REQUIRED_SIZE];
        memset(this->arr, 0, REQUIRED_SIZE * sizeof(uint8_t));
    }
    ~RpcAckBitMap() { delete[] this->arr; }
    void set(uint32_t index) {
        uint32_t bucket = index >> 3;
        this->arr[bucket] |= (1 << (index & 0x7));
    }
    bool get(uint32_t index) {
        uint32_t bucket = index >> 3;
        uint8_t value = this->arr[bucket];
        return (value >> (index & 0x7)) & 1;
    }

   private:
    uint8_t* arr;
    const int REQUIRED_SIZE = 536870912;  // 2^32/8
};

}  // namespace ns3
#endif /* RELIABILITY_H */
