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
 * brief:    this file defines rpc response
 */

#ifndef RPC_RESPONSE_H
#define RPC_RESPONSE_H

//#include <vector>

#include "ns3/RPC.h"

namespace ns3
{
/*
class RpcResponse : public Object
{
public:
    template <typename T>
    RpcResponse(T result)
    {
        Write(result);
    }

    template <typename T>
    T getResult()
    {
        T t;
        index = 0;
        Read(t);
        return t;
    }

private:
    template <typename T>
    void Write(T &t)
    {
        unsigned char len = sizeof(t);
        buffer.push_back(len);
        char *p = reinterpret_cast<char *>(&t);
        for (unsigned char i = 0; i < len; i++)
            buffer.push_back(p[i]);
    }

    template <typename T>
    void Read(T &t)
    {
        unsigned char len = buffer[index++];
        char *d = new char[len];
        for (unsigned char i = 0; i < len; i++)
            d[i] = buffer[index++];
        t = *reinterpret_cast<T *>(&d[0]);
        delete[] d;
    }

    std::vector<char> buffer;
    int index;
};
*/

class RpcResponse : public RPC
{
public:
    RpcResponse(uint32_t size,uint32_t id):RPC(id,size,RPCType::Response){}
}

} // namespace ns3

#endif /* RPC_RESPONSE_H */