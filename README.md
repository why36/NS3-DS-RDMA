
# Introduction
This is NS-3 simulation for RDMA.
*This project is developed based on the simulator of HPCC.*
We implement the transport layer of RDMA.
You can contact Krayecho Yx (<mrgaoxx@smail.nju.edu.cn> (maybe filterd by NJU) / <532820040@qq.com>).

   
## RDMA QPs
### QP Types
   
   - RC: State machine of RDMA RC sender/receiver. (see [IBSpec Vol.1](https://cw.infinibandta.org/document/dl/8567) && [IBSpec Vol.2](https://cw.infinibandta.org/document/dl/8566))
   - UC: State machine of RDMA UC sender/receiver.
### QP Models
   
   - Send Queue: Implemented. Simulates IBV_POST_SEND(IBV_WR);
   - Receive Queue: Not implemented.
   - Completion Queue: Not implemented, immediately returned with a callback for Send/Receive. 
### RDMA Verbs
   - **SEND_WITH_IMM**(**WRITE** is the same as **SEND** in the behaviors of simulation)
## OpenRDMA (A flexible RDMA framework) [The paper is submitting]()
### RPC Framework
### Chunking Module
   - Chunking Module
   - Rtt-based Chunking Implement (Ongoing)
### Congestion Control Module
   - Window Control
   - LEAP-CC (Ongoing)
### Reliability Module
   - Numbering
   - ACK: A simple per-packet ACK mechanism
   - SR: SR

# Layout 
## NS-3 simulation
The ns-3 simulation is under `simulation/`. Refer to the README.md under it for more details.

## Traffic generator (legacy of HPCC)
The traffic generator is under `traffic_gen/`. Refer to the README.md under it for more details.

## Analysis (legacy of HPCC)
We provide a few analysis scripts under `analysis/` to view the packet-level events, and analyzing the fct in the same way as [HPCC](https://liyuliang001.github.io/publications/hpcc.pdf) Figure 11.
Refer to the README.md under it for more details.

# Questions
For technical questions, please create an issue in this repo, so other people can benefit from your questions. 
You can contact Krayecho Yx (<mrgaoxx@smail.nju.edu.cn> (maybe filterd by NJU) / <532820040@qq.com>).
