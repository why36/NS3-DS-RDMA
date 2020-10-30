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
        Ptr<RPC> rpc = Create<RPC>(GetRPCId(), kRequestSize, kResponseSize, RPCType::Request);  // Each client has a unique rpc_id
        m_rpc_request_map.insert(std::pair<uint64_t, Ptr<RPC>>(rpc->rpc_id, rpc));
        NS_LOG_INFO("insert RPC in m_rpc_request_map id: " << std::dec << rpc->rpc_id);
    }
}

void KRPCClient::Start() {
    KRPCInit();
    SendKRPC();
};

void KRPCClient::SendKRPC() {
    NS_LOG_FUNCTION(this);
    for (auto it = m_rpc_request_map.begin(); it != m_rpc_request_map.end(); it++) {
        SendRPC(it->second);
    }
}

void KRPCClient::OnResponseReceived(Ptr<RPC> rpc) {
    NS_LOG_FUNCTION(this);
    // When response is received, it is removed from the Map
    auto it = m_rpc_request_map.find(rpc->rpc_id);
    NS_ASSERT_MSG(it != m_rpc_request_map.end(), "Received an invalid response.");

    m_logger.RecordRPCReceive(it->second);
    m_rpc_request_map.erase(it);
    NS_LOG_INFO("erase RPC in m_rpc_request_map " << it->first);

    while (m_rpc_request_map.size() <= kRPCRequest) {
        Ptr<RPC> rpc = Create<RPC>(GetRPCId(), kRequestSize, kResponseSize, RPCType::Request);
        m_rpc_request_map.insert(std::pair<uint64_t, Ptr<RPC>>(rpc->rpc_id, rpc));
        NS_LOG_INFO("RPC insert into m_rpc_request_map: " << rpc->rpc_id);
        SendRPC(rpc);
    }
}

void KRPCServer::OnRequestReceived(Ptr<RPC> rpc) {
    NS_LOG_FUNCTION(this);
    Ptr<RPC> response = Create<RPC>(rpc->rpc_id, rpc->m_request_size, rpc->m_response_size, RPCType::Response);
    SendRPC(response);
}

}  // Namespace ns3
