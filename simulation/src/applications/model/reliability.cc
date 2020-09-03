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

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/user-space-congestion-control.h"

namespace ns3 {

inline void Reliability::InsertWWR(Ptr<IBVWorkRequest> wr) { rpcImm_verb.push_back(wr); };
inline void Reliability::AckWR(uint32_t imm, uint64_t wr_id) {
    while (rpcImm_verb.front()->wr_id <= wr_id) {
        if (rpcImm_verb.front()->wr_id == wr_id) {
            auto wr =.rpcImm_verb.front();
            m_usc->Retransmit(wr);
        } else {
            rpcImm_verb.pop_front();
            return;
        }
    };

}  // Namespace ns3
