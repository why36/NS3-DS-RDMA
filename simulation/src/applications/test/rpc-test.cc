/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008, 2009 INRIA, UDcast
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
 * Author: yomarisgong <1054087539@qq.com>
 *
 */

#include "ns3/rpc.h"

#include <fstream>

#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/udp-echo-helper.h"
#include "ns3/uinteger.h"
#include <vector>
using namespace ns3;

int add(int a, int b) { return a + b; }

class RPCTestCase : public TestCase {
   public:
    RPCTestCase();
    void init();
    virtual ~RPCTestCase();

   private:
    virtual void DoRun(void);
};

RPCTestCase::RPCTestCase() : TestCase("Test the RPC function, can keep 8 RPCs.") {}

RPCTestCase::~RPCTestCase() {}

static const int kRPCRequest = 8;
static const int interval = 10;

static const int requestSize = 2000;
std::map<uint32_t,RPC> RPCRequestMap;
std::map<uint32_t,RPC>::iterator it;


RPCTestCase::init() {
    for (int i = 0; i < kRPCRequest; i++) {
        RpcRequest req(requestSize);  
        RPCRequestMap.insert(pair<uint32_t,RPC>(req.requestId,req));
    }
}

RPCTestCase::DoRun(void) {
    init();
    //send first 8 rpcs
    DistributedStorageClient client;

    for(it = RPCRequestMap.begin(); it != RPCRequestMap.end(); it++){
      client.SendRpc(it->second);
    }

    int time = 0;
    while(1){
      while(time % interval != 0){
        time++;
      }
      time = 0;
      //keep 8 rpcs

      //When response is received, it is removed from the Map
      it = RPCRequestMap.find(0);
      RPCRequestMap.erase(it);       

      while(RPCRequestMap.size()!=kRPCRequest){
        RpcRequest req(requestSize);  
        RPCRequestMap.insert(pair<uint32_t,RPC>(req.rpc_id,req));
      }
    }
   


    

    
}