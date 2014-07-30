// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__MonitoringThread__
#define __DDS__MonitoringThread__

// BOOST
//#include <boost/date_time/posix_time/ptime.hpp>
// STL
#include <chrono>
#include <functional>
#include <mutex>

namespace dds
{
    class CMonitoringThread
    {
      private:
        CMonitoringThread();
        ~CMonitoringThread();

      public:
        /// \brief Return singleton instance
        static CMonitoringThread& instance();

        void start(const std::function<void(void)>& _idleCallback);
        void updateIdle();

      private:
        void killProcess();

      private:
        std::chrono::steady_clock::time_point m_startTime;
        std::chrono::steady_clock::time_point m_startIdleTime;

        std::function<void(void)> m_idleCallback;

        std::mutex m_mutex; // Mutex for updateIdle call
    };
}

#endif /* defined(__DDS__MonitoringThread__) */
