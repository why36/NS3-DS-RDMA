/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 *
 */

/*
 * author:   Yixiao(Krayecho) Gao <532820040@qq.com>
 * date:     202000707
 */

#ifndef USER_SPACE_CONNECTION_H
#define USER_SPACE_CONNECTION_H

#include <ns3/rdma-queue-pair.h>

#include <map>
#include <queue>
#include <vector>

#include "ns3/application.h"
#include "ns3/chunking.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/rpc.h"
#include "ns3/user-space-congestion-control.h"

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup user-space-connection
 * \class UserSpaceConnection
 * \brief A user-space connection
 *
 */

class SendCompeltionReturnValue;
class RpcResponse;
class RdmaAppQP;
class RdmaAppAckQP;
class Reliability;
class RpcAckBitMap;
class WRidTag;
class RdmaClient;

static const uint32_t ACK_size = 100;

class UserSpaceConnection : public Object {
   public:
    UserSpaceConnection();
    void SendRPC(Ptr<RPC> rpc);

    void OnTxIBVWC(Ptr<IBVWorkCompletion> tx_ibv_wc);
    void OnRxIBVWC(Ptr<IBVWorkCompletion> rx_ibv_wc);

    void SendAck(uint32_t _imm, Ptr<WRidTag> wrid_tag);
    void ReceiveAck(Ptr<IBVWorkRequest> m_ackWc);

    void Retransmit(Ptr<IBVWorkRequest> wc);

    Callback<void, Ptr<RPC>> m_receiveRPCCB;
    void SetReceiveRPCCallback(Callback<void, Ptr<RPC>> cb);

    void StartDequeueAndTransmit();

    void CalculateRTT(Ptr<IBVWorkRequest> wr);

    Ptr<UserSpaceCongestionControl> get_userspace_congestion_control();
    Ptr<RdmaAppQP> get_app_qp();
    void set_app_qp(Ptr<RdmaAppQP> app_qp);

   private:
    void DoSend();
    void SendRetransmissions();
    void SendNewRPC();
    void SendAck();
    void ReceiveAck();

    Ptr<UserSpaceCongestionControl> m_userspace_congestion_control;
    Ptr<ChunkingInterface> m_chunking;
    Ptr<Reliability> m_reliability;

    Ptr<RdmaAppQP> m_app_qp;
    Ptr<RdmaAppAckQP> m_ack_qp;
    std::queue<Ptr<RPC>> m_queuing_rpcs;
    std::queue<Ptr<IBVWorkRequest>> m_retransmissions;
    Ptr<RPC> m_sending_rpc;

    Ptr<RpcAckBitMap> m_rpc_ack_bitmap;
    uint32_t m_remaining_size;
    uint32_t m_receive_ibv_num = 0;
};

inline Ptr<UserSpaceCongestionControl> UserSpaceConnection::get_userspace_congestion_control() { return m_userspace_congestion_control; }

}  // namespace ns3

#endif /* USER_SPACE_CONNECTION */
