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
 */

/*
 * author:   Yixiao(Krayecho) Gao <532820040@qq.com>
 * date:     202000707
 */

#include "ns3/rpc-client-server.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RPCClientServer");
NS_OBJECT_ENSURE_REGISTERED(KRPCClient);
NS_OBJECT_ENSURE_REGISTERED(KRPCServer);
NS_OBJECT_ENSURE_REGISTERED(RPCClient);
NS_OBJECT_ENSURE_REGISTERED(RPCServer);

void KRPCClient::KRPCInit() {
    NS_LOG_FUNCTION(this);
    for (int i = 0; i < kRPCRequest; i++) {
        Ptr<RPC> rpc = Create<RPC>(m_RPCId++, kRequestSize, kResponseSize, RPCType::Request);
        m_RPCRequestMap.insert(std::pair<uint64_t, Ptr<RPC>>(rpc->rpc_id, rpc));
    }
}

void KRPCClient::Start() {
    KRPCInit();
    SendKRPC();
};

void KRPCClient::SendKRPC() {
    NS_LOG_FUNCTION(this);
    for (auto it = m_RPCRequestMap.begin(); it != m_RPCRequestMap.end(); it++) {
        SendRPC(it->second);
    }
}

void KRPCClient::OnResponseReceived(Ptr<RPC> rpc) {
    NS_LOG_FUNCTION(this);
    // When response is received, it is removed from the Map
    auto it = m_RPCRequestMap.find(rpc->rpc_id);
    NS_ASSERT_MSG(it != m_RPCRequestMap.end(), "Received an invalid response.");
    if (it != m_RPCRequestMap.end()) {
        m_logger.RecordRPCReceive(it->second);
        m_RPCRequestMap.erase(it);
    }

    while (m_RPCRequestMap.size() <= kRPCRequest) {
        Ptr<RPC> rpc = Create<RPC>(m_RPCId++, kRequestSize, kResponseSize, RPCType::Request);
        m_RPCRequestMap.insert(std::pair<uint64_t, Ptr<RPC>>(rpc->rpc_id, rpc));
        SendRPC(rpc);
    }
}

void KRPCServer::OnRequestReceived(Ptr<RPC> rpc) {
    NS_LOG_FUNCTION(this);
    Ptr<RPC> response = Create<RPC>(rpc->rpc_id, rpc->m_request_size, rpc->m_response_size, RPCType::Response);
    SendRPC(response);
}

}  // Namespace ns3
