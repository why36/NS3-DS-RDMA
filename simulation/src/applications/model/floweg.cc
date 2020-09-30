
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
#include "floweg.h"

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

// RTTBased chunking
LinearRTTChunking::~LinearRTTChunking(){};

void LinearRTTChunking::UpdateChunkSize(ChunkingSignal &chunkingSignal) { return; };

uint32_t LinearRTTChunking::GetChunkSize(uint32_t window_limit) { return 4096; };

}  // Namespace ns3
