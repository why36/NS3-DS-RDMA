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

#include "ns3/object.h"
#include "ns3/rpc.h"
#include <map>
#include "ns3/simulator.h"
#include "ns3/user-space-connection.h"
namespace ns3 {

class RPCLogger{
    public:
    void RecordRPCReceive(Ptr<RPC> rpc);
    private:
    static const uint8_t kMaxRecordNumber = 100;
    std::array<uint64_t, kMaxRecordNumber> m_rpcLatencies;
    uint8_t m_recordIndex = 0;
    std::ostream& m_output = std::cout;
};

/**
 * \ingroup RpcClient
 * \class RPCClient
 * \brief An rpc client interfance
 *
 */
class RPCClient:public Object{
    public:
    virtual void Start() = 0;
    virtual void OnResponseReceived(Ptr<RPC>) = 0;
    void SendRPC(Ptr<RPC> rpc);
    void SetUSC(Ptr<UserSpaceConnection> usc);
    protected:
    uint64_t m_RPCId = 0;
    std::map<uint64_t, Ptr<RPC>> m_RPCRequestMap;
    Ptr<UserSpaceConnection> m_usc;
    RPCLogger m_logger;
};

class KRPCClient: public RPCClient{
    static const int kInterval = 10;
    static const int kRequestSize = 2000000;
    static const int kResponseSize = 128;
    static const int kRPCRequest = 8;
    void KRPCInit();
    void SendKRPC();
    virtual void Start() override;
    virtual void OnResponseReceived(Ptr<RPC>) override;
};

class RPCServer: public Object{
    public:
    virtual void OnRequestReceived(Ptr<RPC>) = 0;
    void SendRPC(Ptr<RPC> rpc);
    void SetUSC(Ptr<UserSpaceConnection> usc);
    private:
    Ptr<UserSpaceConnection> m_usc;
};

class KRPCServer: public RPCServer{
public:
    virtual void OnRequestReceived(Ptr<RPC>) override;
};



inline void RPCClient::SendRPC(Ptr<RPC> rpc){
    m_usc->SendRPC(rpc);
}
inline void RPCClient::SetUSC(Ptr<UserSpaceConnection> usc){
    m_usc = usc;
}

inline void RPCServer::SendRPC(Ptr<RPC> rpc){
    m_usc->SendRPC(rpc);
}
inline void RPCServer::SetUSC(Ptr<UserSpaceConnection> usc){
    m_usc = usc;
}

inline void RPCLogger::RecordRPCReceive(Ptr<RPC> rpc){
    m_rpcLatencies[m_recordIndex++] = Simulator::Now().GetMicroSeconds() - rpc->m_info.issue_time;
        if (m_recordIndex == kMaxRecordNumber) {
            for (int i = 0; i < kMaxRecordNumber; i++) {
                m_output << m_rpcLatencies[i] << "\n";
            }
            m_recordIndex = 0;
        }
}

}  // namespace ns3

#endif /* RPC_CLIENT_SERVER_H */
