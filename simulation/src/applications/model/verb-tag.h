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

#ifndef VERB_TAG_H
#define VERB_TAG_H

#include "ns3/rpc.h"
#include "ns3/tag.h"

namespace ns3 {

/**
 * \brief This class implements a tag that carries the information about verbs
 * of a SEND_LAST_WITH_IMM packet.
 */

class ChunkSizeTag : public Tag {
   public:
    ChunkSizeTag();
    void SetChunkSize(uint32_t chunkSize);
    uint32_t GetChunkSize(void) const;
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(TagBuffer i) const;
    virtual void Deserialize(TagBuffer i);
    virtual void Print(std::ostream &os) const;

   private:
    uint32_t m_chunkSize;
};

// RPCTag consists of RPCSizeTag and RPCRequestResponseTypeIdTag

class RPCTag : public Tag {
   public:
    RPCTag();
    void SetRequestSize(uint32_t requestSize);
    void SetResponseSize(uint32_t responseSize);
    void SetRPCReqResType(RPCType type);
    void SetRPCReqResId(uint64_t reqres_id);
    uint32_t GetRequestSize(void) const;
    uint32_t GetResponseSize(void) const;
    RPCType GetRPCReqResType(void) const;
    uint64_t GetRPCReqResId(void) const;
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(TagBuffer i) const;
    virtual void Deserialize(TagBuffer i);
    virtual void Print(std::ostream &os) const;

   private:
    uint32_t m_requestSize;
    uint32_t m_responseSize;
    RPCType m_type;
    uint64_t m_reqres_id;
};

class RPCTotalOffsetTag : public Tag {
   public:
    RPCTotalOffsetTag();
    void SetRPCTotalOffset(uint16_t rpcTotalOffest);
    uint16_t GetRPCTotalOffset(void) const;
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(TagBuffer i) const;
    virtual void Deserialize(TagBuffer i);
    virtual void Print(std::ostream &os) const;

   private:
    uint16_t m_rpcTotalOffest;
};

class WRidTag : public Tag {
   public:
    WRidTag();
    void SetWRid(uint64_t wrid);
    uint64_t GetWRid(void) const;
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(TagBuffer i) const;
    virtual void Deserialize(TagBuffer i);
    virtual void Print(std::ostream &os) const;

   private:
    uint64_t m_wrid;
};

}  // namespace ns3

#endif /* VERB_TAG_H */