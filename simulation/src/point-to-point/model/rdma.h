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

#ifndef RDMA_H
#define RDMA_H

#include <array>
#include <functional>

#include "ns3/ipv4-address.h"
#include "ns3/pointer.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"
#include "ns3/verb-tag.h"

namespace ns3 {
enum class IBVerb { IBV_SEND = 0, IBV_WRITE, IBV_SEND_WITH_IMM, IBV_WRITE_WITH_IMM };
// ChunkSizeTag,RPCTag,RPCTotalOffsetTag(optional)
class RdmaQueuePair;

// ChunkSizeTag,RPCTag,RPCTotalOffsetTag(optional)
enum class TagPayloadBit { WRID = 0, CHUNKSIZE, RPCTAG, RPCTOTALOFFSET };

static const uint8_t WRID_BIT = 1 << static_cast<uint8_t>(TagPayloadBit::WRID);
static const uint8_t CHUNKSIZE_BIT = 1 << static_cast<uint8_t>(TagPayloadBit::CHUNKSIZE);
static const uint8_t RPCTAG_BIT = 1 << static_cast<uint8_t>(TagPayloadBit::RPCTAG);
static const uint8_t RPCTOTALOFFSET_BIT = 1 << static_cast<uint8_t>(TagPayloadBit::RPCTOTALOFFSET);

static const uint8_t kGeneralTagPayloadBits = WRID_BIT | CHUNKSIZE_BIT | RPCTAG_BIT;
static const uint8_t kLastTagPayloadBits = kGeneralTagPayloadBits | RPCTOTALOFFSET_BIT;

using TagPayload = struct tag_payload {
    uint8_t mark_tag_bits;
    Ptr<WRidTag> wr_id_tag;
    Ptr<ChunkSizeTag> chunk_size_tag;
    Ptr<RPCTag> rpc_tag;
    Ptr<RPCTotalOffsetTag> rpctotaloffset_tag;
    inline void Serialize(TagBuffer i) const;
    inline void Deserialize(TagBuffer i);
    inline uint32_t GetSerializedSize() const;
};

void TagPayload::Serialize(TagBuffer i) const {
    if (mark_tag_bits & WRID_BIT) {
        wr_id_tag->Serialize(i);
        uint8_t *temp = i.GetMCurrent();
        temp +=  wr_id_tag->GetSerializedSize();
        i.SetMCurrent(temp);
    }
    if (mark_tag_bits & CHUNKSIZE_BIT) {
        chunk_size_tag->Serialize(i);
        uint8_t *temp1 = i.GetMCurrent();
        temp1 +=  chunk_size_tag->GetSerializedSize();
        i.SetMCurrent(temp1);
    }
    if (mark_tag_bits & RPCTAG_BIT) {
        rpc_tag->Serialize(i);
        uint8_t *temp2 = i.GetMCurrent();
        temp2 +=  rpc_tag->GetSerializedSize();
        i.SetMCurrent(temp2);
    }
    if (mark_tag_bits & RPCTOTALOFFSET_BIT) {
        rpctotaloffset_tag->Serialize(i);
        uint8_t *temp3 = i.GetMCurrent();
        temp3 +=  rpctotaloffset_tag->GetSerializedSize();
        i.SetMCurrent(temp3);
    }
}

void TagPayload::Deserialize(TagBuffer i) {
    NS_ASSERT(!wr_id_tag && !chunk_size_tag && !rpc_tag && !rpctotaloffset_tag);
    if (mark_tag_bits & WRID_BIT) {
        wr_id_tag = Create<WRidTag>();
        wr_id_tag->Deserialize(i);
        uint8_t *temp = i.GetMCurrent();
        temp +=  wr_id_tag->GetSerializedSize();
        i.SetMCurrent(temp);
    }
    if (mark_tag_bits & CHUNKSIZE_BIT) {
        chunk_size_tag = Create<ChunkSizeTag>();
        chunk_size_tag->Deserialize(i);
        uint8_t *temp1 = i.GetMCurrent();
        temp1 +=  chunk_size_tag->GetSerializedSize();
        i.SetMCurrent(temp1);
    }
    if (mark_tag_bits & RPCTAG_BIT) {
        rpc_tag = Create<RPCTag>();
        rpc_tag->Deserialize(i);
        uint8_t *temp2 = i.GetMCurrent();
        temp2 +=  rpc_tag->GetSerializedSize();
        i.SetMCurrent(temp2);
    }
    if (mark_tag_bits & RPCTOTALOFFSET_BIT) {
        rpctotaloffset_tag = Create<RPCTotalOffsetTag>();
        rpctotaloffset_tag->Deserialize(i);
        uint8_t *temp3 = i.GetMCurrent();
        temp3 +=  rpctotaloffset_tag->GetSerializedSize();
        i.SetMCurrent(temp3);
    }
}

inline uint32_t TagPayload::GetSerializedSize() const {
    uint32_t size = 0;
    if (mark_tag_bits & WRID_BIT) {
        size += wr_id_tag->GetSerializedSize();
    }
    if (mark_tag_bits & CHUNKSIZE_BIT) {
        size += chunk_size_tag->GetSerializedSize();
    }
    if (mark_tag_bits & RPCTAG_BIT) {
        size += rpc_tag->GetSerializedSize();
    }
    if (mark_tag_bits & RPCTOTALOFFSET_BIT) {
        size += rpctotaloffset_tag->GetSerializedSize();
    }
    return size;
};

class RdmaQueuePair;
using IBVWorkRequest = struct ibv_wr : public SimpleRefCount<ibv_wr> {
    IBVerb verb;
    uint32_t size;
    uint32_t imm;
    TagPayload tags;
    // to do. Krayecho: serialized this
    uint64_t wr_id;

    // for RC QP to obtain accurate RTT,
    // in real we do not use this method
    uint64_t send_completion_time;
    uint64_t last_packet_seq;
    ibv_wr() { tags.mark_tag_bits = kGeneralTagPayloadBits; }
    ibv_wr(int _mark_tag_bits) { tags.mark_tag_bits = _mark_tag_bits; };
};

using IBVWorkCompletion = struct ibv_wc : public SimpleRefCount<ibv_wc> {
    Ptr<RdmaQueuePair> qp;
    IBVerb verb;
    bool isTx;
    uint32_t size;
    uint32_t imm;
    uint64_t completion_time_in_us;
    TagPayload tags;

    Ptr<IBVWorkRequest> wr;
    ibv_wc() { tags.mark_tag_bits = kGeneralTagPayloadBits; };
    ibv_wc(int _mark_tag_bits) { tags.mark_tag_bits = _mark_tag_bits; };
};

// 20200708
// TODO: by now the interface is not incompatible withe Datagram service
enum class QPType { RDMA_RC = 0, RDMA_UC, RDMA_RD, RDMA_UD };

using QPConnectionAttr = struct qp_attr {
    uint16_t pg;
    Ipv4Address sip;
    Ipv4Address dip;
    uint16_t sport;
    uint16_t dport;
    QPType qp_type;
    // to do Krayecho Yx: this is set by directly modify, fix this
    Callback<void, Ptr<IBVWorkCompletion>> completionCB;
    qp_attr();
    qp_attr(uint16_t p_pg, Ipv4Address p_sip, Ipv4Address p_dip, uint16_t p_sport, uint16_t p_dport, QPType p_qp_type);
    qp_attr(const qp_attr&);
    qp_attr(qp_attr& attr);
    qp_attr& operator~();
};

inline QPConnectionAttr::qp_attr() : pg(0), sip(), dip(), sport(0), dport(0), qp_type(QPType::RDMA_RC){};
inline QPConnectionAttr::qp_attr(uint16_t p_pg, Ipv4Address p_sip, Ipv4Address p_dip, uint16_t p_sport, uint16_t p_dport, QPType p_qp_type)
    : pg(p_pg), sip(p_sip), dip(p_dip), sport(p_sport), dport(p_dport), qp_type(p_qp_type){};
inline QPConnectionAttr::qp_attr(const qp_attr& other)
    : pg(other.pg),
      sip(other.sip),
      dip(other.dip),
      sport(other.sport),
      dport(other.dport),
      qp_type(other.qp_type),
      completionCB(other.completionCB){};
inline QPConnectionAttr::qp_attr(qp_attr& other) : qp_attr(reinterpret_cast<const qp_attr&>(other)){};

inline QPConnectionAttr& QPConnectionAttr::operator~() {
    auto temp_sport = sport;
    sport = dport;
    dport = temp_sport;
    auto temp_sip = sip.Get();
    sip.Set(dip.Get());
    dip.Set(temp_sip);
    return *this;
};

using SimpleTuple = struct simple_tuple {
    uint32_t sip;
    uint32_t dip;
    uint16_t sport;
    uint16_t dport;
    uint16_t prio;
};

struct SimpleTupleHash {
    // Krayecho Yx: this is a stupid compress from tuple to uint32_t
    // only contains the slow 7 bits of each
    // we have support over 1024 nodes,  thus at least use 11 bits of ip

    // static const uint32_t IP_BITS = 8;

    std::size_t operator()(const SimpleTuple& s) const {
        uint32_t magic = (2 << 9) - 1;
        return ((s.sip | magic) << 24) + ((s.dip | magic) << 16) + ((static_cast<uint32_t>(s.sport) | magic) << 8) +
               (static_cast<uint32_t>(s.dport) | magic);
    }
};

struct SimpleTupleEqual {
    std::size_t operator()(const SimpleTuple& lhs, const SimpleTuple& rhs) const {
        return ((lhs.sip == rhs.sip) && (lhs.dip == rhs.dip) && (lhs.sport == rhs.sport) && (lhs.dport == rhs.dport) && (lhs.prio == rhs.prio));
    }
};

inline std::ostream& operator<<(std::ostream& os, const SimpleTuple& tuple) {
    os << " sip " << tuple.sip << " dip " << tuple.dip << " sport " << tuple.sport << " dport " << tuple.dport << " prio " << tuple.prio << std::endl;
    return os;
}
inline std::ostream& operator<<(std::ostream& os, SimpleTuple& _tuple) {
    const SimpleTuple& tuple = _tuple;
    os << tuple;
    return os;
}
inline std::ostream& operator<<(std::ostream& os, const QPConnectionAttr& attr) {
    os << " pg " << attr.pg << " sip " << attr.sip << " dip " << attr.dip << " sport " << attr.sport << " dport " << attr.dport << " qp type "
       << static_cast<int>(attr.qp_type) << " callback " << &attr.completionCB << std::endl;
    return os;
}
inline std::ostream& operator<<(std::ostream& os, QPConnectionAttr& _attr) {
    const QPConnectionAttr& attr = _attr;
    os << attr;
    return os;
}

}  // namespace ns3
#endif /* RDMA_H */
