/*
 * author:   krayecho <532820040@qq.com>
 * date:     20201103
 * brief:    rate limiter interface for each priroity queue
 */

#ifndef PRIORITY_Q_PRIORITY_QUEUE_H
#define PRIORITY_Q_PRIORITY_QUEUE_H

#include <queue>

#include "rate_limiter.h"
namespace priority_q {

using Priority = uint8_t;

template <class ElementType>
class PriorityQueue {
   public:
    PriorityQueue(Priority prio);
    bool Enqueue(const Element<ElementType>&& element);
    bool CanDequeue();
    Element<ElementType>&& Dequeue();
    void AddRateLimiter(RateLimiter<ElementType>* limiter);

    uint64_t size();
    uint32_t length();

   private:
    Priority m_prio;
    uint64_t m_size;
    std::queue<RateLimiter<T>*> m_limiters;
    std::queue<Element<T>> m_elements;
};

template <class ElementType>
PriorityQueue<ElementType>::PriorityQueue(Priority prio) : m_prio(prio), m_size(0){};

template <class ElementType>
bool PriorityQueue<ElementType>::Enqueue(const Element<ElementType>&& _element) {
    m_elements.push(_element);
    for (auto limiter : m_limiters) {
        limiter->EnqueueTrigger(_element);
    }
    return true;
};

template <class ElementType>
Element<ElementType>&& PriorityQueue<ElementType>::Dequeue() {
    Element<T> rval = m_elements.front();
    for (auto limiter : m_limiters) {
        limiter->DequeueTrigger(rval);
    }
    m_elements.pop();
    return rval;
};

template <class ElementType>
bool PriorityQueue<ElementType>::CanDequeue() {
    if (m_elements.empty()) {
        return false;
    }

    for (auto limiter : m_limiters) {
        if (limiter->IsLimited()) {
            return false;
        }
    }
    return true;
};

template <class ElementType>
uint64_t PriorityQueue<ElementType>::size() {
    return m_size;
}

template <class ElementType>
uint32_t PriorityQueue<ElementType>::length() {
    return m_elements.size();
}

}  // namespace priority_q

#endif /* PRIORITY_Q_PRIORITY_QUEUE_H */