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

#include <ns3/rdma-queue-pair.h>

#include <map>
#include <queue>
#include <vector>

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/floweg.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
//#include "ns3/rdma-app.h"
#include "ns3/rdma-client.h"
//#include "ns3/reliability.h"
#include "ns3/rpc.h"
#include "ns3/user-space-congestion-control.h"


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
class RdmaAppQP;
class RdmaAppAckQP;
class Reliability;
class RpcAckBitMap;
class WRidTag;
class RdmaClient;

static const uint32_t ACK_size = 100;
// this is a interface used for demonsrate how rdma app works
class SimpleRdmaApp {
    virtual void OnSendCompletion(Ptr<IBVWorkCompletion> completion) = 0;
    virtual void OnReceiveCompletion(Ptr<IBVWorkCompletion> completion) = 0;
};

class UserSpaceConnection : public Object {
   public:
    UserSpaceConnection();
    void SendRPC(Ptr<RPC> rpc);
    Ptr<UserSpaceCongestionControl> m_UCC;
    Ptr<FlowsegInterface> m_flowseg;
    Ptr<Reliability> m_reliability;

    Ptr<RdmaAppQP> m_appQP;
    Ptr<RdmaAppAckQP> m_ackQP;
    std::queue<Ptr<RPC>> m_sendQueuingRPCs;
    std::queue<Ptr<IBVWorkRequest>> m_retransmissions;
    Ptr<RPC> m_sendingRPC;

    // Ptr<IBVWorkRequest> m_ackIbvWr;
    Ptr<RpcAckBitMap> m_rpcAckBitMap;
    uint32_t m_remainingSendingSize;

    // void ReceiveRPC(Ptr<RPC>);
    // std::queue<RPC> m_receiveQueuingRPCs;
    void ReceiveIBVWC(Ptr<IBVWorkCompletion> receiveQueuingIBVWC);
    uint32_t m_receive_ibv_num = 0;

    void SendAck(uint32_t _imm, Ptr<WRidTag> wrid_tag);
    void ReceiveAck(Ptr<IBVWorkCompletion> m_ackWc);

    void Retransmit(Ptr<IBVWorkRequest> wc);

    //Keep 8 rpcs
    static const int kRPCRequest = 8;
    static const int interval = 10;

    static const int requestSize = 2000;

    //Key is request_id and value is RPC. When the response_id received equals request_id, it is removed from the map.
    std::map<uint64_t,Ptr<RPC>> RPCRequestMap;
    std::map<uint64_t,Ptr<RPC>>::iterator it;

    void init();
    void SendKRpc();
    void KeepKRpc(uint64_t response_id);

   private:
    void DoSend();
    void SendRetransmissions();
    void SendNewRPC();
    // void ReceiveRPC();
    //void ReceiveIBVWC();
    void SendAck();
    void ReceiveAck();
};
class DistributedStorageClient : public RdmaClient, public SimpleRdmaApp {
   public:
    static TypeId GetTypeId(void);
    DistributedStorageClient();
    virtual ~DistributedStorageClient();

    /*
     *  public interfaces
     */
    static void Connect(Ptr<DistributedStorageClient> client, Ptr<DistributedStorageClient> server, uint16_t pg);

    // Rdma
    virtual void OnResponse(Ptr<RpcResponse> rpcResponse, Ptr<RdmaQueuePair> qp);
    virtual void OnSendCompletion(Ptr<IBVWorkCompletion> completion) override {};
    virtual void OnReceiveCompletion(Ptr<IBVWorkCompletion> completion) override {};

    
    /*
     *  application interface
     */
    // RPC-level interface
    // void SendRpc(Ptr<RPC> rpc);

    // Used for port Management
    uint16_t GetNextAvailablePort() { return m_port++; };
    //

   protected:
    virtual void DoDispose(void);

   private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void AddQP(Ptr<RdmaAppQP> qp);
    // need maintain a QP collection:
    // somthing like key-value storage <qp,ip>
    // std::hash_map<UserSpaceConnection> m_Connections;
    std::vector<Ptr<UserSpaceConnection>> m_Connections;
    //std::unordered_map<ip,Ptr<UserSpaceConnection>> m_Connections;

    /*
     *  basic attributes
     */
    Ipv4Address m_ip;
    uint16_t m_port = 1;
    //
};

}  // namespace ns3

#endif /* DISTRIBUTED_STORAGE_CLIENT_H */
