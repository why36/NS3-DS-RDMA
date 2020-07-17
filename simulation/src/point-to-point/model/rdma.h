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

#include "ns3/ipv4-address.h"
#include "ns3/pointer.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {
enum class IBVerb { IBV_SEND = 0, IBV_WRITE, IBV_SEND_WITH_IMM, IBV_WRITE_WITH_IMM };

static const int kTagsInWR = 4;
using TagPayload = std::array<Ptr<Tag>, kTagsInWR>;

class RdmaQueuePair;

using IBVWorkRequest = struct ibv_wr {
    IBVerb verb;
    uint32_t size;
    uint32_t imm;
    TagPayload tags;
};

using IBVWorkCompletion = struct ibv_wc {
    Ptr<RdmaQueuePair> qp;
    IBVerb verb;
    uint32_t size;
    uint32_t imm;
    uint64_t time_in_us;
    TagPayload tags;
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
    : pg(other.pg), sip(other.sip), dip(other.dip), sport(other.sport), dport(other.dport), qp_type(other.qp_type){};
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
    uint8_t prio;
    bool operator<(simple_tuple& rhs);
    bool operator=(simple_tuple& rhs);
};

bool SimpleTuple::operator<(SimpleTuple& rhs) {
    return (((static_cast<uint64_t>(sip) << 32) + static_cast<uint64_t>(dip)) <
            ((static_cast<uint64_t>(rhs.sip) << 32) + static_cast<uint64_t>(rhs.dip)))
               ? true
               : (((static_cast<uint64_t>(sport) << 24) + (static_cast<uint64_t>(dport) << 8) + prio) <
                  ((static_cast<uint64_t>(rhs.sport) << 24) + (static_cast<uint64_t>(rhs.dport) << 8) + rhs.prio));
}

bool SimpleTuple::operator=(SimpleTuple& rhs) {
    return ((sip == rhs.sip) && (dip == rhs.dip) && (sport == rhs.sport) && (dport == rhs.dport) && (prio == rhs.prio));
}

}  // namespace ns3
#endif /* RDMA_H */
