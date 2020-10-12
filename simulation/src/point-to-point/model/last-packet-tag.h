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

#ifndef LAST_PACKET_TAG_H
#define LAST_PACKET_TAG_H

#include "ns3/rdma-queue-pair.h"
#include "ns3/tag.h"

namespace ns3 {

/**
 * \brief This class implements a tag that carries the information about verbs
 * of a SEND_LAST_WITH_IMM packet.
 */
class LastPacketTag : public Tag {
   public:
    LastPacketTag();
    void SetIBV_WR(IBVWorkRequest ibv_wr);
    IBVWorkRequest GetIBV_WR(void) const;
    inline static bool HasLastPacketTag(OpCodeOperation op);

    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(TagBuffer i) const;
    virtual void Deserialize(TagBuffer i);
    virtual void Print(std::ostream &os) const;

   private:
    IBVWorkRequest m_ibv_wr;
};

inline bool LastPacketTag::HasLastPacketTag(OpCodeOperation op) {
    return (op == OpCodeOperation::SEND_LAST_WITH_IMM || op == OpCodeOperation::SEND_LAST || op == OpCodeOperation::SEND_ONLY_WITH_IMM ||
            op == OpCodeOperation::SEND_ONLY);
};

}  // namespace ns3

#endif /* LAST_PACKET_TAG_H */