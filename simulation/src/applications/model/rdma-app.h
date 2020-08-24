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

#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/rdma-driver.h"
#include "ns3/rdma-queue-pair.h"
#include "ns3/rpc-response.h"

namespace ns3 {

    class Buffer;

    // this is a interface used for demonsrate how rdma app works
    class SimpleRdmaApp {
        virtual void OnSendCompletion(Ptr<IBVWorkCompletion> completion) = 0;
        virtual void OnReceiveCompletion(Ptr<IBVWorkCompletion> completion) = 0;
    };

    // TO DO: Krayecho Yx: implement more details;
    /**
     * \brief RdmaAppQP is a useable QP interface for application;
     */
    class RdmaAppQP : public Object {
        friend class RdmaCM;

    public:
        RdmaAppQP(Ptr<RdmaDriver> driver, Callback<void, Ptr<RpcResponse>, Ptr<RdmaQueuePair> > OnResponseCB,
            Callback<void,Ptr<IBVWorkCompletion> > OnSendCompletionCB,Callback<void, Ptr<IBVWorkCompletion> > OnReceiveCompletionCB);
        
        /**
         * \brief ibv_post_send;
         * \param IBVWorkRequest wr;
         */
        void PostSend(Ptr<IBVWorkRequest> wr){
            m_qp->ibv_post_send(wr);
        };

    private:
        /**
         * \brief add a qp to this` application, only can be called by RdmaApplicationInstaller;
         * \param create_attr connect attr for this QP;
         */
        void CreateQP(QPCreateAttribute& create_attr);

        void OnCompletion(Ptr<IBVWorkCompletion> completion);
        // TO DO Krayecho Yx:
        // void PostReceive();
        Ptr<RdmaDriver> m_rdmaDriver;
        Ptr<RdmaQueuePair> m_qp;

        /*
         * Callback
         */
        Callback<void, Ptr<IBVWorkCompletion>> m_onSendCompletion;
        Callback<void, Ptr<IBVWorkCompletion>> m_onReceiveCompletion;
        //uint32_t;
    };

    inline void RdmaAppQP::OnCompletion(Ptr<IBVWorkCompletion> completion) {
        if (completion->isTx) {
            m_onSendCompletion(completion);
            return;
        }
        else {
            m_onReceiveCompletion(completion);
            return;
        }
    }

    void RdmaAppQP::CreateQP(QPCreateAttribute& create_attr) {
        m_qp = m_rdmaDriver->AddQueuePair(create_attr);
        //return m_qp;
    };

    //
    // Ensure that each
    class RdmaCM {
    public:
        static int Connect(Ptr<RdmaAppQP> src, Ptr<RdmaAppQP> dst, QPConnectionAttr& srcAttr, QpParam& srcParam, QpParam& dstParam);
    };

    int RdmaCM::Connect(Ptr<RdmaAppQP> src, Ptr<RdmaAppQP> dst, QPConnectionAttr& srcAttr, QpParam& srcParam, QpParam& dstParam) {
        //srcParam.notifyCompletion = src->OnCompletion;
        //dstParam.notifyCompletion = dst->OnCompletion;
        srcParam.notifyCompletion = MakeCallback(&RdmaAppQP::OnCompletion, GetPointer(src));//some mistakes
        dstParam.notifyCompletion = MakeCallback(&RdmaAppQP::OnCompletion, GetPointer(dst));
        QPCreateAttribute src_create_attr(srcAttr, srcParam);
        src->CreateQP(src_create_attr);
        src_create_attr.conAttr.operator~();
        src_create_attr.qpParam = dstParam;
        dst->CreateQP(src_create_attr);
    };


    class RdmaAppAckQP : Object {
        friend class RdmaCM;

    public:
        RdmaAppAckQP(Ptr<RdmaDriver> driver, Callback<void, Ptr<IBVWorkCompletion>> OnSendCompletionCB,
            Callback<void, Ptr<IBVWorkCompletion>> OnReceiveCompletionCB);
        RdmaAppAckQP(Ptr<RdmaDriver> driver);   
        /**
         * \param Packet p;
         */
        //void PostSend(IBVWorkRequest& wr);
        void PostSendAck(Ptr<IBVWorkRequest> wr){    
            m_qp_ack->ibv_post_send(wr);
        }
        //void PostReceiveAck(Ptr<Packet> p);
        uint32_t m_ack_qp_interval;
        uint32_t m_milestone_rx = 0;

    private:
        /**
         * \brief add a qp to this` application, only can be called by RdmaApplicationInstaller;
         * \param create_attr connect attr for this QP;
         */
        void CreateQP(QPCreateAttribute& create_attr)
        {
            m_qp_ack = m_rdmaDriver->AddQueuePair(create_attr);
            //return m_qp_ack;
        }

        //void OnCompletion(Ptr<IBVWorkCompletion> completion);
        // TO DO Krayecho Yx:
        // void PostReceive();
        Ptr<RdmaDriver> m_rdmaDriver;
        Ptr<RdmaQueuePair> m_qp_ack;

        /*
         * Callback
         */
        //Callback<void, Ptr<IBVWorkCompletion>> m_onSendCompletion;
        //Callback<void, Ptr<IBVWorkCompletion>> m_onReceiveCompletion;
    };

}  // namespace ns3

#endif /* RDMA_APP_H */
