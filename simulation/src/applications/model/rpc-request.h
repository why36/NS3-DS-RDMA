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
 * brief:    this file defines rpc request
 */

#ifndef RPC_REQUEST_H
#define RPC_REQUEST_H
/*
#include <string>
#include <tuple>
#include <vector>
*/

#include "ns3/rpc.h"

namespace ns3 {
/*
class RpcRequest : public Object
{
public:
    template <typename... Params>
    RpcRequest(std::string funcName, Params... args)
    {
        WriteString(funcName);

        using ArgsType = std::tuple<typename std::decay<Params>::type...>;
        ArgsType _args = std::make_tuple(args...);
        Write(_args);
    }

    std::string GetFuncName()
    {
        index = 0;
        return ReadString();
    }

    template <typename Tuple, std::size_t... I>
    Tuple GetArgs(std::index_sequence<I...>)
    {
        Tuple t;
        index = 0;
        ReadString(); // function name
        Read((&std::get<I>(t))...);
        return t;
    }

private:
    template <typename... Args>
    void Write(std::tuple<Args...> &t)
    {
        Write(t, std::index_sequence_for<Args...>{});
    }

    template <typename Tuple, std::size_t... Is>
    void Write(Tuple &t, std::index_sequence<Is...>)
    {
        Write(&std::get<Is>(t)...);
    }

    template <typename A, typename... T>
    void Write(A a, T... t)
    {
        Write(a);
        Write(t...);
    }

    template <typename T>
    void Write(T t)
    {
        unsigned char len = sizeof(*t);
        buffer.push_back(len);
        char *p = reinterpret_cast<char *>(t);
        for (unsigned char i = 0; i < len; i++)
            buffer.push_back(p[i]);
    }

    void WriteString(std::string s)
    {
        unsigned char len = s.size();
        buffer.push_back(len);
        for (unsigned char i = 0; i < len; i++)
            buffer.push_back(s[i]);
    }

    template <typename A, typename... T>
    void Read(A a, T... t)
    {
        Read(a);
        Read(t...);
    }

    template <typename T>
    void Read(T t)
    {
        unsigned char len = buffer[index++];
        char *d = new char[len];
        for (unsigned char i = 0; i < len; i++)
            d[i] = buffer[index++];
        *t = *reinterpret_cast<T>(&d[0]);
        delete[] d;
    }

    std::string ReadString()
    {
        unsigned char len = buffer[index++];
        std::string result = "";
        for (unsigned char i = 0; i < len; i++)
            result += buffer[index++];
        return result;
    }

    std::vector<char> buffer;
    int index;
};
*/

class RpcRequest : public RPC {
   public:
    // Requestid is self-augmenting from zero, and is global
    RpcRequest(uint32_t size, uint64_t requestId) : RPC(requestId, size, RPCType::Request) { requestId++; }
    // private:
    // uint64_t requestId;
    // static uint64_t requestId;
};

// uint64_t RpcRequest::requestId = 0;
}  // namespace ns3

#endif /* RPC_REQUEST_H */