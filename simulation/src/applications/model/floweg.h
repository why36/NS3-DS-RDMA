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

#ifndef CHUNKING_H
#define CHUNKING_H

#include "ns3/object.h"
#include "ns3/type-id.h"
#include "ns3/uinteger.h"

namespace ns3 {

enum ChunkingType { DROP_BASED = 0, RTT_BASED, CONGESTION_BASED };

using ChunkingSignal = struct {
    ChunkingType chunking_type;

    // drop-based chunking
    uint32_t chunk_size;
    bool dropped;
    uint64_t now_us;

    // rtt-based chunking
    uint64_t rtt;
    // congestion-based chunking
};

// need a new constructor
class ChunkingInterface : public Object {
   public:
    ChunkingInterface() = delete;
    ChunkingInterface(const ChunkingInterface &) = delete;
    ChunkingInterface(ChunkingType type) {}
    virtual ~ChunkingInterface() = 0;

    virtual uint32_t GetChunkSize(uint32_t window_limit) = 0;
    virtual void UpdateChunkSize(ChunkingSignal &chunkingSignal) = 0;

   private:
    ChunkingType mChunkingType;
};

class DropBasedChunking final : public ChunkingInterface {
   public:
    DropBasedChunking() : ChunkingInterface(ChunkingType::DROP_BASED){};
    ~DropBasedChunking() override;
    uint32_t GetChunkSize(uint32_t window_limit) override;
    void UpdateChunkSize(ChunkingSignal &chunkingSignal) override;
};

class LinearRTTChunking final : public ChunkingInterface {
   public:
    LinearRTTChunking() : ChunkingInterface(ChunkingType::RTT_BASED){};
    ~LinearRTTChunking() override;
    uint32_t GetChunkSize(uint32_t window_limit) override;
    void UpdateChunkSize(ChunkingSignal &chunkingSignal) override;

    uint64_t mMinRTT;
    uint64_t mMaxRTT;
    static uint64_t kSUnitSize;
};

}  // namespace ns3
#endif /* CHUNKING_H */
