/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 Georgia Tech Research Corporation
 *               2007 INRIA
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
 * Authors: yomarisgong<1054087539@qq.com>
 */

#include "last-packet-tag.h"

#include <iomanip>
#include <iostream>

#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("LastPacketTag");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED(LastPacketTag);

LastPacketTag::LastPacketTag() { NS_LOG_FUNCTION(this); }

void LastPacketTag::SetIBV_WR(Ptr<IBVWorkRequest> ibv_wr) {
    m_ibv_wr.verb = ibv_wr->verb;
    m_ibv_wr.size = ibv_wr->size;
    m_ibv_wr.imm = ibv_wr->imm;
    m_ibv_wr.tags = ibv_wr->tags;
    m_ibv_wr.wr_id = ibv_wr->wr_id;
}

IBVWorkRequest LastPacketTag::GetIBV_WR(void) const { return m_ibv_wr; }

TypeId LastPacketTag::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::LastPacketTag").SetParent<Tag>().AddConstructor<LastPacketTag>();
    return tid;
}

TypeId LastPacketTag::GetInstanceTypeId(void) const { return GetTypeId(); }

uint32_t LastPacketTag::GetSerializedSize(void) const {
    uint32_t tag_size = 0;
    return 1 + 4 + 4 + m_ibv_wr.tags.GetSerializedSize() + 8;
}

void LastPacketTag::Serialize(TagBuffer i) const {
    // m_ibv_wr is equal to IBV_SEND_WITH_IMM
    i.WriteU8(m_ibv_wr.tags.mark_tag_bits);
    i.WriteU32(m_ibv_wr.size);
    i.WriteU32(m_ibv_wr.imm);
    m_ibv_wr.tags.Serialize(i);
    i.WriteU64(m_ibv_wr.wr_id);
}

void LastPacketTag::Deserialize(TagBuffer i) {
    // m_ibv_wr is equal to IBV_SEND_WITH_IMM
    m_ibv_wr.tags.mark_tag_bits = i.ReadU8();
    m_ibv_wr.size = i.ReadU32();
    m_ibv_wr.imm = i.ReadU32();
    m_ibv_wr.tags.Deserialize(i);
    m_ibv_wr.wr_id = i.ReadU64();
}

void LastPacketTag::Print(std::ostream &os) const {
    os << "LastPacketTag.size(dec)=" << std::dec << m_ibv_wr.size << " LastPacketLastPacketTag.imm(dec)=" << std::dec << m_ibv_wr.imm;
}

}  // namespace ns3

// TO DO GYY: implement this
