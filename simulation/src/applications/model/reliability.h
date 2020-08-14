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

#ifndef RELIABILITY_H
#define RELIABILITY_H

#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/vector.h"

namespace ns3 {

    using RPCNumber = uint24_t;

    using ACKSeg = struct ack_seg {
        RPCNumber rpc_id;
        uint16_t segment_id;
    }

    class ACK {
        std::vector<ACKSeg> segments;
    }

    class Reliability {
        uint64_t GetMessageNumber() {
            return m_messageNumber++;
        }

        std::list<>
    private:
        Number  m_messageNumber=0;
    };


}  // namespace ns3
#endif /* RELIABILITY_H */
