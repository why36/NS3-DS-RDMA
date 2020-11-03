this is the design draft for prioritized simulation for distributed storage

# an abstraction of NVMe disk
- 1-1 bounded to thread
    -if one thread in charge of 2 SSDs, use doubled value 
- single queuing model
    -latency = queue_length / throughput
- interface
    - Enqueue(priority, rpc)
    - Dequeue(callback)

The following implementation should be of production level
TO DO: encapsure this into libraries

# priority queues
- a group of priority queue with sheduling strategy
- interfaces
    - public interfaces:
        - Enqueue(priority, class T)
        - Dequeue(priority)
        - SetScheduling()
        - SetRateLimiter(priority)
    - private interfaces:
# priority queue
- a single priority queue, with rate limiter
- interfaces
    - public interfaces:
        - Enqueue(class T)
        - Dequeue()
        - SetRateLimiter()
        - length()
        - size()

- TO DO: add flexible container interface
# timer
- a virtual timer class used in rate limiter
    - GetNowInUs()

# rate limiter
- rate limiter for a single priority, IOPS/throughput, etc
    - IsLimited();
    - SetTimer();
    - Update(Priority queue);

# scheduling strategy
- basic class for different kind of scheduling strateties
    - GetNextPriority()
