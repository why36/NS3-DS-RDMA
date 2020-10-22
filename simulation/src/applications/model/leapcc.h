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
 * Author: Krayecho(Yixiao) Gao <532820040@qq.com>
 *
 */

#ifndef LEAPCC_H
#define LEAPCC_H

#include "ns3/data-rate.h"
#include "ns3/object.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/user-space-congestion-control.h"
namespace ns3 {

class LeapCCSignal : public RTTSignal {
   public:
    uint32_t mBytesAcked;
};

class LeapCC : public RttWindowCongestionControl {
   public:
    LeapCC();

    static TypeId GetTypeId(void);
    static double CalculateBeta(double max_rtt, uint64_t min_cwnd, uint64_t max_cwnd);
    static double CalculateGamma(double beta, uint64_t max_cwnd);

    virtual void UpdateSignal(CongestionSignal* signal) override;
    virtual uint32_t GetCongestionWindow() override;

    void AjustRTT();
    double CalculateUtilization(double new_rtt);
    uint64_t CalulateNewWindow(double utilization, uint32_t bytes_acked);
    void SetLeapCCDefaultParameter();
    uint64_t LimiteCongestionWindow(uint64_t updateWindow, uint64_t tmpWindow);
    void OnPacketLost();
    void OnRetransmissionTimeout();

    static uint64_t kMinWindow;
    static uint64_t kMTUSize;
    static uint64_t kAbnormalRTTThreshold;

    uint64_t mMinWindow;
    uint64_t mMaxWindow;

    double mBaseRTT;
    double mMaxRTT;
    double mTargetRTT;
    double mUtilization;
    double mLasttestRTT;
    double mAvgRTT;
    double mAlpha;
    DataRate mNICRate;

    uint8_t mIncStage;
    uint8_t mMaxStage;
    uint8_t mHAI;
    uint64_t mLastDecreaseTime;
    double mBeta;
    double mGamma;

   private:
    static const uint64_t kDefaultMinWinwodw = 4096;
    static const uint64_t kDefaultMTUSize = 1024;
    static const uint64_t kDefaultAbnormalRTTThreshold = 2000;
};

inline uint64_t LeapCC::LimiteCongestionWindow(uint64_t updateWindow, uint64_t tmpWindow) {
    tmpWindow = std::max(updateWindow, static_cast<uint64_t>(m_window_in_bytes * 1.0 / 2));
    tmpWindow = std::min(tmpWindow, mMaxWindow);
    tmpWindow = std::max(tmpWindow, mMinWindow);
    return tmpWindow;
};

inline void LeapCC::AjustRTT() {
    mTargetRTT = mBaseRTT + std::max(1.0, mBeta * 1.0 / sqrt(m_window_in_bytes * 1.0 / kMTUSize));
    mTargetRTT = std::max(mTargetRTT, mBaseRTT);
    mTargetRTT = std::min(mTargetRTT, mMaxRTT);
}

inline double LeapCC::CalculateBeta(double max_rtt, uint64_t min_cwnd, uint64_t max_cwnd) {
    return max_rtt * 1.0 / (1.0 / sqrt(min_cwnd * 1.0 / kMTUSize) - 1.0 / sqrt(max_cwnd * 1.0 / kMTUSize));
};

inline double LeapCC::CalculateGamma(double beta, uint64_t max_cwnd) { return -beta * 1.0 / sqrt(max_cwnd * 1.0 / kMTUSize); };

}  // namespace ns3
#endif /* LEAPCC_H */
