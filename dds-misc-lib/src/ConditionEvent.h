// Copyright 2014 GSI, Inc. All rights reserved.
//
//
#ifndef _DDS_CONDITIONEVENT_H_
#define _DDS_CONDITIONEVENT_H_

// STD
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace dds::misc
{
    /**
     * @brief Helper class for conditional events.
     **/
    class CConditionEvent
    {
      public:
        CConditionEvent()
            : m_bFlag(false)
        {
        }

        void wait()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [&]() { return m_bFlag; });
        }

        template <class Rep, class Period>
        bool waitFor(const std::chrono::duration<Rep, Period>& _relTime)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_condition.wait_for(lock, _relTime, [&]() { return m_bFlag; });
        }

        template <class Clock, class Duration>
        bool waitUntil(const std::chrono::time_point<Clock, Duration>& _timeoutTime)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_condition.wait_until(lock, _timeoutTime, [&]() { return m_bFlag; });
        }

        void notifyAll()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_bFlag = true;
            }
            m_condition.notify_all();
        }

        void notifyOne()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_bFlag = true;
            }
            m_condition.notify_one();
        }

        void reset()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_bFlag = false;
        }

      private:
        bool m_bFlag;
        std::mutex m_mutex;
        std::condition_variable m_condition;
    };
}; // namespace dds::misc
#endif /*_DDS_CONDITIONEVENT_H_*/
