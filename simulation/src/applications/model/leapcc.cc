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

#include "ns3/leapcc.h"

#include "ns3/assert.h"
#include "ns3/double.h"
#include "ns3/log.h"

namespace ns3 {

uint64_t LeapCC::kMinWindow = LeapCC::kDefaultMinWinwodw;
uint64_t LeapCC::kMTUSize = LeapCC::kDefaultMTUSize;
uint64_t LeapCC::kAbnormalRTTThreshold = LeapCC::kDefaultAbnormalRTTThreshold;

LeapCC::LeapCC() : mUtilization(0), mAvgRTT(0), mIncStage(0), mLastDecreaseTime(0){};

TypeId LeapCC::GetTypeId() {
    static TypeId tid =
        TypeId("ns3::LeapCC")
            .SetParent<UserSpaceCongestionControl>()
            .AddConstructor<LeapCC>()
            .AddAttribute("LeapBaseRTT", "base rtt in LeapCC", DoubleValue(50.0), MakeDoubleAccessor(&LeapCC::mBaseRTT), MakeDoubleChecker<double>())
            .AddAttribute("TargetRTT", "target rtt in LeapCC", DoubleValue(50.0), MakeDoubleAccessor(&LeapCC::mTargetRTT),
                          MakeDoubleChecker<double>())
            .AddAttribute("NICRate", "NIC rate in LeapCC", DataRateValue(DataRate("25Gbps")), MakeDataRateAccessor(&LeapCC::mNICRate),
                          MakeDataRateChecker())
            .AddAttribute("MinCongestionWindow", "minimal congestion window in LeapCC", UintegerValue(kMTUSize),
                          MakeUintegerAccessor(&LeapCC::mMinWindow), MakeUintegerChecker<uint64_t>())
            .AddAttribute("MaxCongestionWindow", "maximal congestion window in LeapCC", UintegerValue(64 * kMTUSize),
                          MakeUintegerAccessor(&LeapCC::mMaxWindow), MakeUintegerChecker<uint64_t>())
            .AddAttribute("CongestionWindow", "congestion window in LeapCC", UintegerValue(8 * kMTUSize),
                          MakeUintegerAccessor(&LeapCC::m_window_in_bytes), MakeUintegerChecker<uint64_t>())
            .AddAttribute("Alpha", "alpha in LeapCC", DoubleValue(0.8), MakeDoubleAccessor(&LeapCC::mAlpha), MakeDoubleChecker<double>())
            .AddAttribute("MaxStage", "minimal congestion window in LeapCC", UintegerValue(5), MakeUintegerAccessor(&LeapCC::mMaxStage),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("MaxRTT", "maximal rtt in LeapCC", DoubleValue(300.0), MakeDoubleAccessor(&LeapCC::mMaxRTT), MakeDoubleChecker<double>())
            .AddAttribute("Beta", "beta in LeapCC", DoubleValue(CalculateBeta(300.0, kMTUSize, 64 * kMTUSize)), MakeDoubleAccessor(&LeapCC::mBeta),
                          MakeDoubleChecker<double>())
            .AddAttribute("Gamma", "gamma in LeapCC", DoubleValue(CalculateGamma(CalculateBeta(300.0, kMTUSize, 64 * kMTUSize), 64 * kMTUSize)),
                          MakeDoubleAccessor(&LeapCC::mGamma), MakeDoubleChecker<double>());
    return tid;
}

void LeapCC::UpdateSignal(CongestionSignal* signal) {
    LeapCCSignal* leap_signal = reinterpret_cast<LeapCCSignal*>(signal);
    if (leap_signal->m_rtt > kAbnormalRTTThreshold) return;
    AjustRTT();
    double cur_utilization = CalculateUtilization(leap_signal->m_rtt);
    uint64_t updated_window = CalulateNewWindow(cur_utilization, leap_signal->mBytesAcked);
    m_window_in_bytes = LimiteCongestionWindow(updated_window, m_window_in_bytes);
    return;
};

double LeapCC::CalculateUtilization(double rtt) {
    double u = 0;
    double lastRTT = mAvgRTT;
    mAvgRTT = (1 - mAlpha) * 1.0 * mAvgRTT + mAlpha * 1.0 * rtt;
    if (mAvgRTT <= mTargetRTT) {
        u = 0;
        return u;
    } else {
        u = (mAvgRTT - mTargetRTT) * 1.0 / mTargetRTT;
    }
    return u;
}

uint64_t LeapCC::CalulateNewWindow(double utilization, uint32_t bytes_acked) {
    uint64_t rtnWindow = 0;
    if (utilization > 0) {
        rtnWindow = m_window_in_bytes * std::max(0.5, 1 - utilization);
        mIncStage = 0;
    } else if (utilization == 0) {
        double increased_cwnd = kMTUSize;
        if (mIncStage < mMaxStage) {
            increased_cwnd = (2 * kMTUSize * 1.0 / m_window_in_bytes) * bytes_acked;
            rtnWindow = m_window_in_bytes + increased_cwnd;
            mIncStage++;
        } else {
            increased_cwnd = (mHAI * kMTUSize * 1.0 / m_window_in_bytes) * bytes_acked;
            rtnWindow = m_window_in_bytes + increased_cwnd;
        }
    }
    return rtnWindow;
};

uint32_t LeapCC::GetCongestionWindow() { return m_window_in_bytes; };
void LeapCC::OnPacketLost() { m_window_in_bytes = mMinWindow; }
void LeapCC::OnRetransmissionTimeout() { m_window_in_bytes = mMinWindow; }
}  // namespace ns3
