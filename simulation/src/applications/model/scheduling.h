/*
 * author:   krayecho <532820040@qq.com>
 * date:     20201103
 * brief:    rate limiter interface for each priroity queue
 */

#ifndef PRIORITY_Q_SHCEDULING_H
#define PRIORITY_Q_SHCEDULING_H

#include <queue>

#include "priority_queues.h"

namespace priority_q {

using SchedulePolicy = enum class enum_schedule_policy { RR = 0, SP = 1, WFQ = 2 };

template <class ElementType>
class Scheduling {
    Scheduling(SchedulePolicy schedule_type);
    virtual Priority GetNextPriority() = 0;
    virtual void EnqueueTrigger() = 0;
    virtual void DequeueTrigger() = 0;
    void set_queues(PriorityQueues<ElementType>*);

   private:
    SchedulePolicy m_type;
    PriorityQueues<ElementType>* m_queues;
};

template <class ElementType>
class RRScheduling : public Scheduling<ElementType> {
   public:
    RRScheduling();
    virtual Priority GetNextPriority() override;

   private:
    Priority m_last_prio;
};

template <class ElementType>
class SPScheduling : public Scheduling<ElementType> {
   public:
    SPScheduling();
    virtual Priority GetNextPriority() override;

   private:
};

using WFQTokens = std::array<uint8_t, kMaxPriorities>;

template <class ElementType>
class WFQScheduling : public Scheduling<ElementType> {
   public:
    WFQScheduling();
    virtual Priority GetNextPriority() override;
    void set_tokens(WFQTokens&& token);

   private:
    Priority m_last_prio;
    uint8_t m_last_token;
    WFQTokens m_tokens;
};

template <class ElementType>
void Scheduling<ElementType>::set_queues(PriorityQueues<ElementType>* queues) {
    m_queues = queue;
};

template <class ElementType>
RRScheduling<ElementType>::RRScheduling() : Scheduling(SchedulePolicy::RR), m_last_prio(0) {}

template <class ElementType>
Priority RRScheduling<ElementType>::GetNextPriority() {
    for (auto count = m_queues->get_max_priority; i > 0; i--) {
        m_last_prio++;
        if (m_queues.GetQueue(m_last_prio)->CanDequeue()) {
            return m_last_prio;
        }
    };
    return kNullPriority;
}

template <class ElementType>
SPScheduling<ElementType>::SPScheduling() : Scheduling(SchedulePolicy::SP) {}

template <class ElementType>
Priority SPScheduling<ElementType>::GetNextPriority() {
    for (Priority priority = 0; i < m_queues->get_max_prio(); i++) {
        if (m_queues.GetQueue(priority)->CanDequeue()) {
            return priority;
        }
    };
    return kNullPriority;
}

template <class ElementType>
WFQScheduling<ElementType>::WFQScheduling() : Scheduling(SchedulePolicy::WFQ), m_last_prio(0), m_last_token(0) {}

template <class ElementType>
void WFQScheduling<ElementType>::set_tokens(WFQTokens&& token) {
    m_tokens = std::move(token);
};

template <class ElementType>
Priority WFQScheduling<ElementType>::GetNextPriority() {
    for (auto count = m_queues->get_max_priority; i > 0; i--) {
        if (m_queues.GetQueue(m_last_prio)->CanDequeue()) {
            ++m_last_tokens;
            if (m_tokens[m_last_prio] == m_last_tokens) {
                m_last_token = 0;
                ++m_last_prio;
            }
            return m_last_prio;
        }
        ++m_last_prio;
    };
    return kNullPriority;
}

}  // namespace priority_q

#endif /* PRIORITY_Q_SHCEDULING_H */