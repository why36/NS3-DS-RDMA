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

#ifndef USER_SPACE_CONGESTION_CONTROL_H
#define USER_SPACE_CONGESTION_CONTROL_H

#include "ns3/uinteger.h"

namespace ns3 {

class CongestionSignal {
    // To do
};

enum CongestionControlType {
    WINDOW_BASE = 0 << 0,
    RATE_BASE = 1 << 0,
    RTT_SIGNAL = 0 << 1,
    ECN_SIGNAL = 1 << 1
}

class UserSpaceCongestionControl {
   public:
    UserSpaceCongestionControl() = delete;
    CongestionControlType GetCongestionContorlType() { return mType; };

    void UpdateSignal(CongestionSignal& signal);

   private:
    CongestionControlType mType;
};

class WindowCongestionControl : public UserSpaceCongestionControl {
   public:
    virtual void UpdateSignal(CongestionSignal& signal);
    virtual uint32_t GetCongestionWindow() = 0;

   private:
    // forbids to construct
    WindowCongestionControl() { mType |= WINDOW_BASE; }
}

class RttWindowCongestionControl : public object,
                                   public WindowCongestionControl {
   public:
    RttWindowCongestionControl() : WindowCongestionControl() { mType |= RTT_SIGNAL; }
}

inline CongestionControlType
UserSpaceCongestionControl::GetCongestionContorlType() {
    return mType;
};

}  // namespace ns3
#endif /* USER_SPACE_CONGESTION_CONTROL_H */
