/*
 * author:   krayecho <532820040@qq.com>
 * date:     20201103
 * brief:    rate limiter interface for each priroity queue
 */

#ifndef PRIORITY_Q_PRIORITY_QUEUES_H
#define PRIORITY_Q_PRIORITY_QUEUES_H

#include <map>
#include <queue>

#include "priority_queue.h"
namespace priority_q {

constexpr Priority kMaxPriorities = 8;
constexpr Priority kNullPriority = UINT8_MAX;

using PriorityEndian = enum class enum_priority_endian {
    SMALL = 0,  // means that lower priority is larger
    BIG = 1
};

constexpr PriorityEndian kUsingPriorityEndian = PriorityEndian::SMALL;

template <class ElementType>
class Scheduling;

template <class ElementType>
class PriorityQueues {
   public:
    PriorityQueues(Priority max_prio);
    void Enqueue(Priority prio, Element<ElementType>&& element);
    void Enqueue(Priority prio, Element<ElementType>& element);
    void Dequeue(Priority prio, Element<ElementType>&& element);
    void Dequeue(Priority prio, Element<ElementType>& element);
    Prioity get_max_prio();

   private:
    Scheduling<ElementType>* m_scheduling;
    std::array<PriorityQueue<ElementType>, kMaxPriorities> m_queues;
    uint8_t m_max_prio;
};

template <class ElementType>
PriorityQueues<ElementType>::PriorityQueues(Priority max_prio) : m_max_prio(max_prio) {}

}  // namespace priority_q

#endif /* PRIORITY_Q_PRIORITY_QUEUES_H */