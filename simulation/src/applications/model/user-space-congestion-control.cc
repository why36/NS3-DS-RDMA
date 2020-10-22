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

#include "ns3/user-space-congestion-control.h"

#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3 {

bool WindowCongestionControl::IncreaseInflight(uint32_t size) {
    m_inflight_in_bytes += size;
    if (m_window_in_bytes <= m_inflight_in_bytes) {
        m_throttled = true;
    }
    return m_throttled;
}

bool WindowCongestionControl::DecreaseInflight(uint32_t size) {
    NS_ASSERT(m_inflight_in_bytes >= size);
    m_inflight_in_bytes -= size;
    if (m_inflight_in_bytes < m_window_in_bytes) {
        m_throttled = false;
    }
    return m_throttled;
}

uint32_t WindowCongestionControl::GetAvailableSize() {
    if (m_throttled) {
        return 0;
    } else {
        NS_ASSERT(m_window_in_bytes > m_inflight_in_bytes);
        return m_window_in_bytes - m_inflight_in_bytes;
    }
}

}  // namespace ns3
