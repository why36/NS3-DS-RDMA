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

#include "ns3/object.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
namespace ns3 {

enum class CongestionControlSignalType { RTT_SIGNAL = 0, ECN_SIGNAL = 1 };

enum class CongestionControlPacingType { WINDOW_BASE = 0, RATE_BASE = 1 };

class CongestionSignal {
   public:
    CongestionSignal(CongestionControlSignalType _type);
    CongestionControlSignalType m_type;
};

class RTTSignal : public CongestionSignal {
   public:
    RTTSignal();
    uint8_t m_rtt;
};

using CCType = struct cctype {
    CongestionControlSignalType signal_type;
    CongestionControlPacingType pacing_type;
    cctype();
    cctype(CongestionControlSignalType signal_t);
    cctype(CongestionControlPacingType pacing_t);
    cctype(CongestionControlSignalType signal_t, CongestionControlPacingType pacing_t);
    bool operator==(cctype c);
};

static const CCType RTTWindowCCType = CCType(CongestionControlSignalType::RTT_SIGNAL, CongestionControlPacingType::WINDOW_BASE);

class UserSpaceCongestionControl : public Object {
   public:
    UserSpaceCongestionControl(CongestionControlSignalType _signal_type);
    UserSpaceCongestionControl(CongestionControlPacingType _pacing_type);
    UserSpaceCongestionControl(CongestionControlSignalType _signal_type, CongestionControlPacingType _pacing_type);

    CCType GetCongestionContorlType();
    virtual void UpdateSignal(CongestionSignal* signal) = 0;

   protected:
    CCType m_type;
};

class WindowCongestionControl : public UserSpaceCongestionControl {
   public:
    virtual void UpdateSignal(CongestionSignal* signal) = 0;
    virtual uint32_t GetCongestionWindow() = 0;
    uint32_t GetAvailableSize();
    bool IncreaseInflight(uint32_t size);
    bool DecreaseInflight(uint32_t size);

   protected:
    // forbids to construct
    WindowCongestionControl();
    WindowCongestionControl(CongestionControlSignalType _signal_type);
    uint32_t m_window_in_bytes;
    uint32_t m_inflight_in_bytes;
    bool m_throttled;
};

class RttWindowCongestionControl : public WindowCongestionControl {
   public:
    RttWindowCongestionControl() : WindowCongestionControl(CongestionControlSignalType::RTT_SIGNAL) {}
    virtual void UpdateSignal(CongestionSignal* signal) = 0;
    virtual uint32_t GetCongestionWindow() = 0;
};

inline CongestionSignal::CongestionSignal(CongestionControlSignalType _type) : m_type(_type) {}

inline RTTSignal::RTTSignal() : CongestionSignal(CongestionControlSignalType::RTT_SIGNAL) {}

inline CCType::cctype() {}
inline CCType::cctype(CongestionControlSignalType signal_t) : signal_type(signal_t) {}
inline CCType::cctype(CongestionControlPacingType pacing_t) : pacing_type(pacing_t) {}
inline CCType::cctype(CongestionControlSignalType signal_t, CongestionControlPacingType pacing_t) : signal_type(signal_t), pacing_type(pacing_t) {}
inline bool CCType::operator==(cctype c) { return (c.signal_type == signal_type && c.pacing_type == pacing_type); }

inline UserSpaceCongestionControl::UserSpaceCongestionControl(CongestionControlSignalType _signal_type) {
    m_type.signal_type = _signal_type;
    m_type.pacing_type = CongestionControlPacingType::WINDOW_BASE;
}
inline UserSpaceCongestionControl::UserSpaceCongestionControl(CongestionControlPacingType _pacing_type) {
    m_type.signal_type = CongestionControlSignalType::RTT_SIGNAL;
    m_type.pacing_type = _pacing_type;
}
inline UserSpaceCongestionControl::UserSpaceCongestionControl(CongestionControlSignalType _signal_type, CongestionControlPacingType _pacing_type) {
    m_type.signal_type = _signal_type;
    m_type.pacing_type = _pacing_type;
}
inline CCType UserSpaceCongestionControl::GetCongestionContorlType() { return m_type; };

inline WindowCongestionControl::WindowCongestionControl()
    : UserSpaceCongestionControl(CongestionControlPacingType::WINDOW_BASE), m_window_in_bytes(4096), m_inflight_in_bytes(0) {}
inline WindowCongestionControl::WindowCongestionControl(CongestionControlSignalType _signal_type)
    : UserSpaceCongestionControl(_signal_type, CongestionControlPacingType::WINDOW_BASE), m_window_in_bytes(4096), m_inflight_in_bytes(0) {}

}  // namespace ns3
#endif /* USER_SPACE_CONGESTION_CONTROL_H */
