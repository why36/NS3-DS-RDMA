*-*-Mode : C++;
c - file - style : "gnu";
indent - tabs - mode : nil;
- * -* /
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

#include "verb-tag.h"

#include <iomanip>
#include <iostream>

#include "ns3/log.h"
#include "ns3/tag.h"

NS_LOG_COMPONENT_DEFINE("VerbTag");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(FlowSegSizeTag);

FlowSegSizeTag::FlowSegSizeTag() { NS_LOG_FUNCTION(this); }

void FlowSegSizeTag::SetFlowSegSize(uint32_t flowSegSize) { m_flowSegSize = flowSegSize; }

uint32_t FlowSegSizeTag::GetFlowSegSize() { return m_flowSegSize; }

TypeId FlowSegSizeTag::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::FlowSegSizeTag").SetParent<Tag>().AddConstructor<FlowSegSizeTag>();
    return tid;
}

TypeId FlowSegSizeTag::GetInstanceTypeId(void) const { return GetTypeId(); }

uint32_t FlowSegSizeTag::GetSerializedSize(void) const { return sizeof(uint32_t); }

void FlowSegSizeTag::Serialize(TagBuffer i) const { i.WriteU32(m_flowSegSize); }

void FlowSegSizeTag::Deserialize(TagBuffer i) { m_flowSegSize = i.ReadU32(); }

void FlowSegSizeTag::Print(std::ostream &os) const { os << "FlowSegSizeTag.flowSegSize(dec)=" << std::dec << m_flowSegSize; }

NS_OBJECT_ENSURE_REGISTERED(RPCSizeTag);

RPCSizeTag::RPCSizeTag() { NS_LOG_FUNCTION(this); }

void RPCSizeTag::SetRPCSize(uint32_t rpcSize) { m_rpcSize = rpcSize; }

uint32_t RPCSizeTag::GetRPCSize(void) const { return m_rpcSize; }

TypeId RPCSizeTag::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::RPCSizeTag").SetParent<Tag>().AddConstructor<RPCSizeTag>();
    return tid;
}

TypeId RPCSizeTag::GetInstanceTypeId(void) const { return GetTypeId(); }

uint32_t RPCSizeTag::GetSerializedSize(void) const { return sizeof(uint32_t); }

void RPCSizeTag::Serialize(TagBuffer i) const { i.WriteU32(m_rpcSize); }

void RPCSizeTag::Deserialize(TagBuffer i) { m_rpcSize = i.ReadU32(); }

void RPCSizeTag::Print(std::ostream &os) const { os << "RPCSizeTag.m_rpcSize(dec)=" << std::dec << m_rpcSize; }


NS_OBJECT_ENSURE_REGISTERED(RPCTotalOffsetTag);

RPCTotalOffsetTag::RPCTotalOffsetTag() { NS_LOG_FUNCTION(this); }

void RPCTotalOffsetTag::SetRPCSize(uint16_t rpcTotalOffest) { m_rpcTotalOffest = rpcTotalOffest; }

uint16_t RPCTotalOffsetTag::GetRPCSize(void) const { return m_rpcTotalOffest; }

TypeId RPCTotalOffsetTag::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::RPCTotalOffsetTag").SetParent<Tag>().AddConstructor<RPCTotalOffsetTag>();
    return tid;
}

TypeId RPCTotalOffsetTag::GetInstanceTypeId(void) const { return GetTypeId(); }

uint32_t RPCTotalOffsetTag::GetSerializedSize(void) const { return sizeof(uint16_t); }

void RPCTotalOffsetTag::Serialize(TagBuffer i) const { i.WriteU16(m_rpcTotalOffest); }

void RPCTotalOffsetTag::Deserialize(TagBuffer i) { m_rpcTotalOffest = i.ReadU16(); }

void RPCTotalOffsetTag::Print(std::ostream &os) const { os << "RPCTotalOffsetTag.m_rpcTotalOffest(dec)=" << std::dec << m_rpcTotalOffest; }

}  // namespace ns3

// TO DO GYY: implement this
