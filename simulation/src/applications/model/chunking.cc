
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
#include "chunking.h"

#include "ns3/assert.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("Chunking");
namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED(DropBasedChunking);
NS_OBJECT_ENSURE_REGISTERED(LinearRTTChunking);

ChunkingInterface::~ChunkingInterface() {}

// delete function
// DropBasedChunking::DropBasedChunking() {}

DropBasedChunking::~DropBasedChunking() {}

void DropBasedChunking::UpdateChunkSize(ChunkingSignal &chunkingSignal) { return; };

uint32_t DropBasedChunking::GetChunkSize(uint32_t window_limit) { return 4096; };

uint64_t LinearRTTChunking::kChunkingUnitSize = 4096;
uint64_t LinearRTTChunking::kMaxUnitNum = 16;

TypeId LinearRTTChunking::GetTypeId() {
    static TypeId tid = TypeId("LinearRTTChunking")
                            .AddConstructor<LinearRTTChunking>()
                            .SetParent<ChunkingInterface>()
                            .AddAttribute("MinRTT", "minimal rtt in LinearRTTChunking", UintegerValue(50),
                                          MakeUintegerAccessor(&LinearRTTChunking::mMinRTT), MakeUintegerChecker<uint64_t>())
                            .AddAttribute("MaxRTT", "maximal rtt in LinearRTTChunking", UintegerValue(300),
                                          MakeUintegerAccessor(&LinearRTTChunking::mMaxRTT), MakeUintegerChecker<uint64_t>());
    //.AddAttribute("ChunkingUnitSize", "chunking unit size in LinearRTTChunking", UintegerValue(4096),
    //              MakeUintegerAccessor(&LinearRTTChunking::kChunkingUnitSize), MakeUintegerChecker<uint64_t>())
    //.AddAttribute("MaxUnitNum", "maximal number of unit size of chunking in LinearRTTChunking", UintegerValue(16),
    //              MakeUintegerAccessor(&LinearRTTChunking::kMaxUnitNum), MakeUintegerChecker<uint64_t>());

    return tid;
}

// RTTBased chunking
LinearRTTChunking::~LinearRTTChunking(){};

void LinearRTTChunking::UpdateChunkSize(ChunkingSignal &chunkingSignal) {
    if (chunkingSignal.rtt <= mMinRTT) {
        mCurrentChunkSize = kChunkingUnitSize * kMaxUnitNum;
        return;
    }
    mCurrentChunkSize = kMaxUnitNum - kMaxUnitNum * (chunkingSignal.rtt - mMinRTT) / (mMaxRTT - mMinRTT);
    return;
};

uint32_t LinearRTTChunking::GetChunkSize(uint32_t window_limit) {
    return mCurrentChunkSize < window_limit ? mCurrentChunkSize : (window_limit / kMaxUnitNum * kMaxUnitNum);
};

}  // Namespace ns3
