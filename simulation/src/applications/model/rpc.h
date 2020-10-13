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
 * author:   yomarisgong <1054087539@qq.com>
 * date:     202000724
 * brief:    this file defines rpc
 */

#ifndef RPC_H
#define RPC_H

#include <ns3/object.h>

#include "ns3/uinteger.h"
namespace ns3 {
/*
class RPC : public Object
{
public:
    uint32_t m_rpc_size;
    uint32_t rpc_id; //24 bit
    //uint16_t segment_id = 0;
    template <typename F>
    void Bind(std::string name, F func)
    {
        funcMap[name] = std::bind(&RPC::Execute<F>, this, func, std::placeholders::_1);
    }

    RpcResponse Send(RpcRequest request) { return funcMap[request.GetFuncName()](request); }

private:
    template <typename F>
    RpcResponse Execute(F func, RpcRequest request)
    {
        return Execute_(func, request);
    }

    template <typename R, typename... Params>
    RpcResponse Execute_(R (*func)(Params...), RpcRequest requst)
    {
        using ArgsType = std::tuple<typename std::decay<Params>::type...>;
        constexpr auto N = std::tuple_size<typename std::decay<ArgsType>::type>::value;

        ArgsType args = requst.GetArgs<ArgsType>(std::make_index_sequence<N>{});

        auto f = std::function<R(Params...)>(func);
        R r = Invoke(f, args, std::make_index_sequence<N>{});
        RpcResponse response = RpcResponse(r);
        return response;
    }

    template <typename R, typename... Params, typename Tuple, std::size_t... Index>
    R Invoke(std::function<R(Params... ps)> &func, Tuple &t, std::index_sequence<Index...>)
    {
        return func(std::get<Index>(t)...);
    }
    std::map<std::string, std::function<RpcResponse(RpcRequest)>> funcMap;
};
*/
enum class RPCType { Request = 0, Response = 1 };

using RPCInfo = struct rpc_info {
    uint64_t issue_time;
    uint64_t send_completion_time;
};

class RPC : public Object {
   public:
    uint32_t rpc_id;
    uint32_t m_request_size;
    uint32_t m_response_size;
    uint64_t m_reqres_id;  // request or response id
    RPCType m_rpc_type;
    RPCInfo m_info;
    RPC();
    RPC(uint64_t id, uint32_t request_size,uint32_t response_size, RPCType type) : m_reqres_id(id), m_request_size(request_size),m_response_size(response_size),m_rpc_type(type) {}
};

}  // namespace ns3

#endif /* RPC_H */