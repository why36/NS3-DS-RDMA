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
 * date:     202000716
 * brief:    this file defines the rdma application interface
 */

#ifndef RDMA_APP_H
#define RDMA_APP_H

#include "ns3/ptr.h"
#include "ns3/rdma-driver.h"
#include "ns3/rdma-queue-pair.h"
#include "ns3/rpc.h"
#include "ns3/user-space-connection.h"

namespace ns3 {

class Buffer;

// TO DO: Krayecho Yx: implement more details;
/**
 * \brief RdmaAppQP is a useable QP interface for application;
 */
class RdmaCM;
class UserSpaceConnection;

class RdmaAppQP : public Object {
    friend class RdmaCM;

   public:
    RdmaAppQP(Ptr<RdmaDriver> driver, Callback<void, Ptr<IBVWorkCompletion>> on_send_completion_cb,
              Callback<void, Ptr<IBVWorkCompletion>> on_receive_completion_cb);
    void CreateQP(QPCreateAttribute& create_attr);

    void PostSend(Ptr<IBVWorkRequest> wr);
    void OnCompletion(Ptr<IBVWorkCompletion> completion);

    QPType GetQPType();
    void set_userspace_connection(Ptr<UserSpaceConnection> usc);
    Ptr<RdmaDriver> get_rdma_driver();

    Ptr<RdmaDriver> m_rdma_driver;
    Ptr<UserSpaceConnection> m_userspace_connection;
    Ptr<RdmaQueuePair> m_qp;
    Callback<void, Ptr<IBVWorkCompletion>> m_on_send_completion;
    Callback<void, Ptr<IBVWorkCompletion>> m_on_receive_completion;
};

class RdmaAppAckQP : public Object {
    friend class RdmaCM;

   public:
    RdmaAppAckQP(Ptr<RdmaDriver> driver, Callback<void, Ptr<IBVWorkCompletion>> on_send_completion_cb,
                 Callback<void, Ptr<IBVWorkCompletion>> on_receive_completion_cb);
    RdmaAppAckQP(Ptr<RdmaDriver> driver);

    void CreateQP(QPCreateAttribute& create_attr);

    void PostSendAck(Ptr<IBVWorkRequest> wr);

    Ptr<RdmaDriver> m_rdma_driver;
    Ptr<RdmaQueuePair> m_qp_ack;
    uint32_t m_ack_qp_interval;
    uint32_t m_milestone_rx = 0;
};

class RdmaCM {
   public:
    inline static int Connect(Ptr<RdmaAppQP> src, Ptr<RdmaAppQP> dst, QPConnectionAttr& srcAttr);
};

inline RdmaAppQP::RdmaAppQP(Ptr<RdmaDriver> driver, Callback<void, Ptr<IBVWorkCompletion>> on_send_completion_cb,
                            Callback<void, Ptr<IBVWorkCompletion>> on_receive_completion_cb) {
    m_rdma_driver = driver;
    m_on_send_completion = on_send_completion_cb;
    m_on_receive_completion = on_receive_completion_cb;
}

inline void RdmaAppQP::OnCompletion(Ptr<IBVWorkCompletion> completion) {
    if (completion->isTx) {
        m_on_send_completion(completion);
        return;
    } else {
        m_on_receive_completion(completion);
        return;
    }
}

inline void RdmaAppQP::CreateQP(QPCreateAttribute& create_attr) { m_qp = m_rdma_driver->AddQueuePair(create_attr); }

inline void RdmaAppQP::PostSend(Ptr<IBVWorkRequest> wr) { m_qp->ibv_post_send(wr); }

inline QPType RdmaAppQP::GetQPType(void) { return m_qp->GetQPType(); }

inline Ptr<RdmaDriver> RdmaAppQP::get_rdma_driver() { return m_rdma_driver; }

inline void RdmaAppQP::set_userspace_connection(Ptr<UserSpaceConnection> usc) { m_userspace_connection = usc; }

inline void RdmaAppAckQP::CreateQP(QPCreateAttribute& create_attr) { m_qp_ack = m_rdma_driver->AddQueuePair(create_attr); }

inline void RdmaAppAckQP::PostSendAck(Ptr<IBVWorkRequest> wr) { m_qp_ack->ibv_post_send(wr); }

inline int RdmaCM::Connect(Ptr<RdmaAppQP> src, Ptr<RdmaAppQP> dst, QPConnectionAttr& srcAttr) {
    srcAttr.completionCB = MakeCallback(&RdmaAppQP::OnCompletion, GetPointer(src));
    QPCreateAttribute src_create_attr(srcAttr);
    src->CreateQP(src_create_attr);
    src_create_attr.conAttr.operator~();
    src_create_attr.conAttr.completionCB = MakeCallback(&RdmaAppQP::OnCompletion, GetPointer(dst));
    dst->CreateQP(src_create_attr);
};

}  // namespace ns3

#endif /* RDMA_APP_H */
