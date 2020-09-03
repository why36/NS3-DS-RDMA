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

#include "ns3/pointer.h"
#include "ns3/uinteger.h"

namespace ns3 {

enum class CongestionControlSignalType { RTT_SIGNAL = 0, ECN_SIGNAL = 1 };

enum class CongestionControlPacingType { WINDOW_BASE = 0, RATE_BASE = 1 };

class CongestionSignal {
   public:
    CongestionControlSignalType mType;
    CongestionSignal(CongestionControlSignalType _type) : mType(_type) {}
};

class RTTSignal : public CongestionSignal {
   public:
    RTTSignal() : CongestionSignal(CongestionControlSignalType::RTT_SIGNAL) {}
    uint8_t mRtt;
};

using CCType = struct cctype {
    CongestionControlSignalType signalType;
    CongestionControlPacingType pacingType;
    cctype() {}
    cctype(CongestionControlSignalType signal_t) : signalType(signal_t) {}
    cctype(CongestionControlPacingType pacing_t) : pacingType(pacing_t) {}
    cctype(CongestionControlSignalType signal_t, CongestionControlPacingType pacing_t) : signalType(signal_t), pacingType(pacing_t) {}
};

static const CCType RTTWindowCCType = CCType(CongestionControlSignalType::RTT_SIGNAL, CongestionControlPacingType::WINDOW_BASE);

class UserSpaceCongestionControl {
   public:
    // UserSpaceCongestionControl() = delete;
    UserSpaceCongestionControl(CongestionControlPacingType _pacingType) { mType.pacingType = _pacingType; }

    UserSpaceCongestionControl(CongestionControlSignalType _signalType) { mType.signalType = _signalType; }

    CCType GetCongestionContorlType() { return mType; };

    virtual void UpdateSignal(CongestionSignal& signal) = 0;

   protected:
    CCType mType;
};

class WindowCongestionControl : public UserSpaceCongestionControl {
   public:
    virtual void UpdateSignal(CongestionSignal& signal);
    virtual uint32_t GetCongestionWindow();
    uint32_t GetAvailableSize();
    bool IncreaseInflight(uint32_t size);
    bool DecreaseInflight(uint32_t size);

   protected:
    // forbids to construct
    WindowCongestionControl() : UserSpaceCongestionControl(CongestionControlPacingType::WINDOW_BASE), mWindow(0), mInflight(0) {}
    WindowCongestionControl(CongestionControlSignalType _signalType) : UserSpaceCongestionControl(_signalType) {}
    uint32_t mWindow;
    uint32_t mInflight;
    bool mThrottled;
};

bool WindowCongestionControl::IncreaseInflight(uint32_t size) {
    mInflight += size;
    if (mWindow <= mInflight) {
        mThrottled = true;
    }
    return mThrottled;
}

bool WindowCongestionControl::DecreaseInflight(uint32_t size) {
    NS_ASSERT(mInflight >= size);
    mInflight -= size;
    if (mInflight < mWindow) {
        mThrottled = false;
    }
    return mThrottled;
}

uint32_t WindowCongestionControl::GetAvailableSize() {
    if (mThrottled) {
        return 0;
    } else {
        NS_ASSERT(mWindow > mInflight);
        return mWindow - mInflight;
    }
};

class RttWindowCongestionControl : public Object, public WindowCongestionControl {
   public:
    RttWindowCongestionControl() : WindowCongestionControl(CongestionControlSignalType::RTT_SIGNAL) {}
    virtual void UpdateSignal(CongestionSignal& signal) = 0;
    virtual uint32_t GetCongestionWindow() = 0;
};

class LeapCC : public RttWindowCongestionControl {
   public:
    LeapCC();
    virtual void UpdateSignal(CongestionSignal& signal) override;
    virtual uint32_t GetCongestionWindow() override;
}

}  // namespace ns3
#endif /* USER_SPACE_CONGESTION_CONTROL_H */
