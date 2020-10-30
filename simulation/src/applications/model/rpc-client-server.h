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
 * Author:   Yixiao(Krayecho) Gao <532820040@qq.com>
 */

#ifndef RPC_CLIENT_SERVER_H
#define RPC_CLIENT_SERVER_H

#include <fstream>
#include <map>

#include "ns3/object.h"
#include "ns3/rpc.h"
#include "ns3/simulator.h"
#include "ns3/user-space-connection.h"
namespace ns3 {

class RPCLogger {
   public:
    RPCLogger();
    void RecordRPCReceive(Ptr<RPC> rpc);

   private:
    static const uint8_t kMaxRecordNumber = 100;
    std::array<uint64_t, kMaxRecordNumber> m_rpcLatencies;
    uint8_t m_recordIndex = 0;
    std::ofstream m_output;
};

/**
 * \ingroup RpcClient
 * \class RPCClient
 * \brief An rpc client interfance
 *
 */
class RPCClient : public Object {
   public:
    RPCClient();
    virtual void Start() = 0;
    virtual void OnResponseReceived(Ptr<RPC>) = 0;
    void SendRPC(Ptr<RPC> rpc);
    uint64_t GetRPCId();
    void set_userspace_connection(Ptr<UserSpaceConnection> userspace_connection);

   protected:
    uint64_t m_rpc_id;
    std::map<uint64_t, Ptr<RPC>> m_rpc_request_map;
    Ptr<UserSpaceConnection> m_userspace_connection;
    RPCLogger m_logger;
};

class KRPCClient : public RPCClient {
    static const int kInterval = 10;
    static const int kRequestSize = 1000;
    static const int kResponseSize = 128;
    static const int kRPCRequest = 8;
    void KRPCInit();
    void SendKRPC();
    virtual void Start() override;
    virtual void OnResponseReceived(Ptr<RPC>) override;
};

class RPCServer : public Object {
   public:
    virtual void OnRequestReceived(Ptr<RPC>) = 0;
    void SendRPC(Ptr<RPC> rpc);
    void set_userspace_connection(Ptr<UserSpaceConnection> userspace_connection);

   private:
    Ptr<UserSpaceConnection> m_userspace_connection;
};

class KRPCServer : public RPCServer {
   public:
    virtual void OnRequestReceived(Ptr<RPC>) override;
};

inline RPCClient::RPCClient() : m_rpc_id(0){};
inline uint64_t RPCClient::GetRPCId() { return m_rpc_id++; };
inline void RPCClient::SendRPC(Ptr<RPC> rpc) { m_userspace_connection->SendRPC(rpc); }
inline void RPCClient::set_userspace_connection(Ptr<UserSpaceConnection> userspace_connection) {
    m_userspace_connection = userspace_connection;
    m_userspace_connection->SetReceiveRPCCallback(MakeCallback(&RPCClient::OnResponseReceived, this));
}

inline void RPCServer::SendRPC(Ptr<RPC> rpc) { m_userspace_connection->SendRPC(rpc); }
inline void RPCServer::set_userspace_connection(Ptr<UserSpaceConnection> userspace_connection) {
    m_userspace_connection = userspace_connection;
    m_userspace_connection->SetReceiveRPCCallback(MakeCallback(&RPCServer::OnRequestReceived, this));
}

inline RPCLogger::RPCLogger() {
    m_output.open("./rpc_latency.log", std::ios::out);
    if (!m_output.is_open()) {
        std::cout << "RPCLogger： cannot open file for RPC logging." << std::endl;
    }
};

inline void RPCLogger::RecordRPCReceive(Ptr<RPC> rpc) {
    m_rpcLatencies[m_recordIndex++] = Simulator::Now().GetMicroSeconds() - rpc->m_info.issue_time;
    if (m_recordIndex == kMaxRecordNumber) {
        for (int i = 0; i < kMaxRecordNumber; i++) {
            m_output << m_rpcLatencies[i] << "\n";
        }
        m_output << std::endl;
        m_recordIndex = 0;
    }
}

}  // namespace ns3

#endif /* RPC_CLIENT_SERVER_H */
