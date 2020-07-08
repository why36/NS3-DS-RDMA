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

#ifndef FLOWSEG_H
#define FLOWSEG_H

#include "ns3/uinteger.h"

namespace ns3 {

enum FlowsegType { DROP_BASED = 0, RTT_BASED, CONGESTION_BASED };

using FlowsegSignal = struct {
    FlowsegType flowseg_type;

    // drop-based flowseg
    uint32_t segment_size;
    bool dropped;
    uint64_t now_us;

    // rtt-based flowseg
    // congestion-based flowseg
};

class FlowsegInterface {
   public:
    FlowsegInterface() = delete;
    FlowsegInterface(const FlowsegInterface&) = delete;
    virtual ~FlowsegInterface() = 0;

    virtual uint32_t GetSegSize() = 0;
    virtual void UpdateSeg(FlowsegSignal& flowsegSignal) = 0;

   private:
    FlowsegType mFlowsegType;
};

class DropBasedFlowseg : public Object, public FlowsegInterface {
   public:
    static GetTypeId() { return tid; }
    DropBasedFlowseg();
    ~DropBasedFlowseg() override;
    uint32_t GetSegSize() override;
    void UpdateSeg(FlowsegSignal& flowsegSignal) override;
} final;

}  // namespace ns3
#endif /* FLOWSEG_H */
