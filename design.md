# distributed storage daemon
   - Contains many threads, full-mesh connections among threads of different applications
   - rpcs are sent via (srcapp,thread_id,dst_app,thread_id) 
# distributed storage thread
   - Each thread listens one specific port
   - Each thread has one RPCClient/Server than connects to other threads
# rpc design
   - RPCClient/Server: basic class for RPC send/receive
      - maintains USCs to different RPC servers
   - KRPCClientServer: an implementation of RPCClient/Server that maintains K inflight RPCs 
# user-space connection design
   -
# rdma-app QP 
   -
# QueuePair 
   -
# statistic informations
   - RPC level:
      - carried in each rpc: latency
   - USC level:
      - drop ratio
	  - segment size distribution
