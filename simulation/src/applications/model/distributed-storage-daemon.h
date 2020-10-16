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

#ifndef DISTRIBUTED_STORAGE_DAEMON_H
#define DISTRIBUTED_STORAGE_DAEMON_H

#include <ns3/rdma-queue-pair.h>

#include <map>
#include <queue>
#include <vector>

#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/rpc-client-server.h"

namespace ns3 {

class DistributedStorageDaemon;
class DistributedStorageThread : public Object {
   public:
    explicit DistributedStorageThread(uint16_t port);
    virtual ~DistributedStorageThread(){};
    void Start();
    uint16_t GetPort();
    void AddRPCClient(Ptr<RPCClient> client);
    void AddRPCServer(Ptr<RPCServer> server);

   private:
    std::vector<Ptr<RPCClient>> m_clients;
    std::vector<Ptr<RPCServer>> m_servers;

    uint16_t m_port;
    Ptr<DistributedStorageDaemon> m_daemon;
};

/**
 * \ingroup distributedStorage
 * \class DistributedStorageDaemon
 * \brief A distributed storage client.
 *
 */
class DistributedStorageDaemon : public Application {
   public:
    static void Connect(Ptr<DistributedStorageDaemon> client, uint32_t client_thread, Ptr<DistributedStorageDaemon> server, uint32_t server_thread,
                        uint16_t pg);

    static TypeId GetTypeId(void);

    DistributedStorageDaemon();
    virtual ~DistributedStorageDaemon();

    void AddThread(int i);
    Ptr<DistributedStorageThread> GetThread(int i);

    void setIp(Ipv4Address ip);
    Ipv4Address getIp();

   protected:
    virtual void DoDispose(void);

   private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    uint16_t GetNextAvailablePort();

    std::vector<Ptr<DistributedStorageThread>> m_threads;
    Ipv4Address m_ip;
    uint16_t m_port;
};

inline uint16_t DistributedStorageThread::GetPort() { return m_port; }

inline void DistributedStorageDaemon::setIp(Ipv4Address ip) { m_ip = ip; }

inline Ipv4Address DistributedStorageDaemon::getIp() { return m_ip; }

inline void DistributedStorageDaemon::AddThread(int i) {
    NS_ASSERT(i == m_threads.size());
    m_threads.push_back(Create<DistributedStorageThread>(GetNextAvailablePort()));
}

inline Ptr<DistributedStorageThread> DistributedStorageDaemon::GetThread(int i) { return m_threads[i]; }

inline uint16_t DistributedStorageDaemon::GetNextAvailablePort() { return m_port++; }

}  // namespace ns3

#endif /* DISTRIBUTED_STORAGE_CLIENT_H */
