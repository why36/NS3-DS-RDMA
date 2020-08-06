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
#include "ns3/pointer.h"

namespace ns3 {

    class CongestionSignal {
    public:
        CongestionControlSignalType mType;
    };


    class RTTSignal :public CongestionSignal {
    public:
        RTTSignal() :mType(CongestionControlSignalType::RTT_SIGNAL) {};
        uint8_t mRtt;
    };

    enum class CongestionControlSignalType {
        RTT_SIGNAL = 0,
        ECN_SIGNAL = 1
    };

    enum class CongestionControlPacingType {
        WINDOW_BASE = 0,
        RATE_BASE = 1
    };

    using CongestionControlType = struct cctype {
        CongestionControlSignalType signalType;
        CongestionControlPacingType pacingType;
        CongestionControlType(CongestionControlSignalType signal_t, CongestionControlPacingType pacing_t) :signalType(signal_t), pacingType(pacing_t) {};
    };

    class UserSpaceCongestionControl {
    public:
        UserSpaceCongestionControl() = delete;
        CongestionControlType GetCongestionContorlType() {
            return mType;
        };

        virtual void UpdateSignal(CongestionSignal& signal)=0;
    protected:
        CongestionControlType mType;
    };

    inline CongestionControlType
        UserSpaceCongestionControl::GetCongestionContorlType() {
        return mType;
    };

    class WindowCongestionControl : public UserSpaceCongestionControl {
    public:
        virtual void UpdateSignal(CongestionSignal& signal);
        virtual uint32_t GetCongestionWindow();

    protected:
        // forbids to construct
        WindowCongestionControl() {
            mType.pacingType = CongestionControlPacingType::WINDOW_BASE;
        }
    };

    class RttWindowCongestionControl : public Object,
        public WindowCongestionControl {
    public:
        RttWindowCongestionControl() : WindowCongestionControl() {
            mType.signalType = CongestionControlSignalType::RTT_SIGNAL;
        }

        void UpdateSignal(CongestionSignal& signal)=0;
        virtual uint32_t GetCongestionWindow() = 0;
    };


    class RttWindowCongestionControl : public Object,
        public WindowCongestionControl {
    public:
        RttWindowCongestionControl() : WindowCongestionControl() {
            mType.signalType = CongestionControlSignalType::RTT_SIGNAL;
        }

        void UpdateSignal(CongestionSignal& signal)=0;
        virtual uint32_t GetCongestionWindow() = 0;
    };


}  // namespace ns3
#endif /* USER_SPACE_CONGESTION_CONTROL_H */
