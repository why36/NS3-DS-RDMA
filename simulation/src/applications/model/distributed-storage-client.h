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

#ifndef DISTRIBUTED_STORAGE_CLIENT_H
#define DISTRIBUTED_STORAGE_CLIENT_H

#include <ns3/rdma.h>

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/flowseg.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/user-space-congestion-control.h"
#include "ns3/reliability.h"

namespace ns3 {

    class Socket;
    class Packet;

    /**
     * \ingroup distributedStorage
     * \class DistributedStorageClient
     * \brief A distributed storage client.
     *
     */

    class SendCompeltionReturnValue;
    class RpcResponse;

    class UserSpaceConnection {
    public:
        void SendRPC(Ptr<RPC>);
        Ptr<UserSpaceCongestionControl> m_UCC;
        Ptr<FlowsegInterface> m_flowseg;
        Ptr<RdmaAppQP> m_appQP;
        Ptr<RdmaAppAckQP> m_ackQP;
        std::queue<Ptr<RPC>> m_sendQueuingRPCs;
        Ptr<RPC> m_sendingRPC;
        Ptr<Reliability> m_reliability;
        RpcAckBitMap m_rpcAckBitMap;
        uint32_t m_remainingSendingSize;

        //void ReceiveRPC(Ptr<RPC>);
        //std::queue<RPC> m_receiveQueuingRPCs;
        void ReceiveIBVWC(Ptr<IBVWorkCompletion> receiveQueuingIBVWC);
        std::queue<Ptr<IBVWorkCompletion>> m_receiveQueuingIBVWCs;
        Ptr<IBVWorkCompletion> m_receivingIBVWC;
        uint32_t m_receive_ibv_num = 0;

    private:
        void SendRPC();
        //void ReceiveRPC();
        void ReceiveIBVWC();
    };
    class DistributedStorageClient : public RdmaClient, public SimpleRdmaApp {
    public:
        static TypeId GetTypeId(void);
        DistributedStorageClient();
        virtual ~DistributedStorageClient();

        /*
         *  public interfaces
         */
        static void Connect(Ptr<DistributedStorageClient> client, Ptr<DistributedStorageClient> server, uint16_t pg, uint32_t size);

        // Rdma
        virtual void OnResponse(Ptr<RpcResponses> rpcResponse, Ptr<RdmaQueuePair> qp) override;
        virtual void OnSendCompletion(Ptr<IBVWorkCompletion> completion) override;
        virtual void OnReceiveCompletion(Ptr<IBVWorkCompletion> completion) override;

        /*
         *  application interface
         */
         // RPC-level interface
        void SendRpc(uint32_t size);

        // Used for port Management
        void GetNextAvailablePort();
        //

    protected:
        virtual void DoDispose(void);

    private:
        virtual void StartApplication(void);
        virtual void StopApplication(void);

        void AddQP(Ptr<RdmaAppQP> qp);
        // need maintain a QP collection:
        // somthing like key-value storage <qp,ip>
        std::hash_map<UserSpaceConnection> m_Connections;

        /*
         *  basic attributes
         */
        Ipv4Address m_ip;
        //
    };

}  // namespace ns3

#endif /* DISTRIBUTED_STORAGE_CLIENT_H */
