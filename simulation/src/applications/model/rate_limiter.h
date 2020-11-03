/*
 * author:   krayecho <532820040@qq.com>
 * date:     20201103
 * brief:    rate limiter interface for each priroity queue
 */

#ifndef PRIORITY_Q_RATE_LIMITER_H
#define PRIORITY_Q_RATE_LIMITER_H
#include "timer.h"

namespace priority_q {
template <class ElementType>
class PriorityQueue;

template <class ElementType>
class Element {
   public:
    GetSize();
    Element(ElementType);
    virtual ~Element();

   private:
    T m_element;
};

template <class ElementType>
Element<ElementType>::Element(ElementType _element) : m_element(_element) {}

template <class ElementType>
class RateLimiter {
   public:
    RateLimiter();
    virtual ~RateLimiter();
    virtual bool IsLimited();

    virtual void EnqueueTrigger(const Element<ElementType>&& enqueue_element);
    void EnqueueTrigger(const Element<ElementType>& enqueue_element);
    virtual void DequeueTrigger(const Element<ElementType>&& dequeu_element);
    void DequeueTrigger(const Element<ElementType>& dequeu_element);
    void set_queue(PriorityQueue<ElementType>* queue);

   private:
    PriorityQueue* m_queue;
    Timer* timer;
};

template <class ElementType>
void RateLimiter<ElementType>::set_queue(PriorityQueue<ElementType>* queue) {
    m_queue = queue;
}

template <class ElementType>
void RateLimiter<ElementType>::EnqueueTrigger(const Element<ElementType>& enqueue_element) {
    std::cout << "Calling EnqueueTrigger of left value" << std::endl;
    EnqueueTrigger(enqueue_element);
}

template <class ElementType>
void RateLimiter<ElementType>::DequeueTrigger(const Element<ElementType>& dequeue_element) {
    std::cout << "Calling DequeueTrigger of left value" << std::endl;
    DequeueTrigger(dequeue_element);
}

}  // namespace priority_q

#endif /* PRIORITY_Q_RATE_LIMITER_H */