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

#include <functional>
#include <map>
#include <string>

#include "rpc-request.h"
#include "rpc-response.h"

namespace ns3 {

class Rpc {
   public:
    template <typename F>
    void Bind(std::string name, F func) {
        funcMap[name] = std::bind(&Rpc::Execute<F>, this, func, std::placeholders::_1);
    }

    RpcResponse Send(RpcRequset request) { return funcMap[request.GetFuncName()](request); }

   private:
    template <typename F>
    RpcResponse Execute(F func, RpcRequest requst) {
        return Execute_(func, requst);
    }

    template <typename R, typename... Params>
    RpcResponse Execute_(R (*func)(Params...), RpcRequest requst) {
        using ArgsType = std::tuple<typename std::decay<Params>::type...>;
        constexpr auto N = std::tuple_size<typename std::decay<ArgsType>::type>::value;

        ArgsType args = requst.GetArgs<ArgsType>(std::make_index_sequence<N>{});

        auto f = std::function<R(Params...)>(func);
        R r = Invoke(f, args, std::make_index_sequence<N>{});
        RpcResponse response = RpcResponse(r);
        return response;
    }

    template <typename R, typename... Params, typename Tuple, std::size_t... Index>
    R Invoke(std::function<R(Params... ps)> &func, Tuple &t, std::index_sequence<Index...>) {
        return func(std::get<Index>(t)...);
    }

    std::map<std::string, std::function<RpcResponse(RpcRequest)>> funcMap;
};

}  // namespace ns3

#endif /* RPC_H */