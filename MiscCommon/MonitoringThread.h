// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__MonitoringThread__
#define __DDS__MonitoringThread__

// DDS
#include "Logger.h"
#include "Process.h"
// STL
#include <chrono>
#include <functional>
#include <thread>
#include <mutex>
// API
#include <csignal>

namespace dds
{
    class CMonitoringThread
    {
        typedef std::function<bool()> callbackFunction_t;

      private:
        CMonitoringThread()
        {
        }
        ~CMonitoringThread()
        {
        }

      public:
        /// \brief Return singleton instance
        static CMonitoringThread& instance()
        {
            static CMonitoringThread instance;
            return instance;
        }

        /// \brief Main function user has to run to start monitoring thread.
        /// \param[in] _idleTime Maximum allowed elapsed time since last activity in seconds.
        /// \param[in] _idleCallback Function which is called after idle is detected.
        /// \example CMonitoringThread::instance().start(300, [](){ do_something_here() });
        void start(double _idleTime, const std::function<void(void)>& _idleCallback)
        {
            static const float LOOP_TIME_DELAY = 5.f;
            static const float WAITING_TIME = 20.f;

            m_startIdleTime = std::chrono::steady_clock::now();

            std::thread t(
                [this, &_idleCallback, _idleTime]()
                {
                    try
                    {
                        while (true)
                        {
                            std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

                            // Check if process is idle.
                            std::chrono::seconds idleTime =
                                std::chrono::duration_cast<std::chrono::seconds>(currentTime - m_startIdleTime);

                            // Call registred callback functions
                            // We use Erase-remove idiom to execute callback and remove expired if needed.
                            m_registeredCallbackFunctions.erase(remove_if(
                                                                    m_registeredCallbackFunctions.begin(),
                                                                    m_registeredCallbackFunctions.end(),
                                                                    [&](callbackFunction_t& i)
                                                                    {
                                                                        // A callback function can return
                                                                        // false, which
                                                                        // means it wants to be unregistered
                                                                        // (expire)
                                                                        bool bActive = i();
                                                                        return !bActive;
                                                                    }),
                                                                m_registeredCallbackFunctions.end());

                            if (idleTime.count() > _idleTime)
                            {
                                // First call idle callback
                                LOG(MiscCommon::info) << "The process is idle for " << idleTime.count()
                                                      << " sec. Call idle callback.";
                                _idleCallback();
                                sleep(WAITING_TIME);

                                // Call terminate
                                LOG(MiscCommon::info) << "The process is idle for " << idleTime.count()
                                                      << " sec. Terminate the process.";
                                std::terminate();
                                sleep(WAITING_TIME);

                                // Kill process
                                LOG(MiscCommon::info) << "The process is idle for " << idleTime.count()
                                                      << " sec. Kill the process.";
                                killProcess();
                            }

                            sleep(LOOP_TIME_DELAY);
                        }
                    }
                    catch (std::exception& _e)
                    {
                        LOG(MiscCommon::error) << "MonitoringThread exception: " << _e.what();
                    }
                });
            t.detach();
        }

        void updateIdle()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_startIdleTime = std::chrono::steady_clock::now();
        }

        void registerCallbackFunction(callbackFunction_t _handler)
        {
            m_registeredCallbackFunctions.push_back(_handler);
        }

      private:
        void killProcess()
        {
            pid_t pidToKill(::getpid());
            if (pidToKill > 0 && MiscCommon::IsProcessExist(pidToKill))
            {
                LOG(MiscCommon::log_stdout) << " self exiting (" << pidToKill << ")...";
                // TODO: Maybe we need more validations of the process before
                // sending a signal. We don't want to kill someone else.
                kill(pidToKill, SIGTERM);

                // Waiting for the process to finish
                size_t iter(0);
                const size_t max_iter = 30;
                while (iter <= max_iter)
                {
                    if (!MiscCommon::IsProcessExist(pidToKill))
                    {
                        LOG(MiscCommon::log_stdout) << std::endl;
                        break;
                    }
                    LOG(MiscCommon::log_stdout) << ".";
                    sleep(1); // sleeping for 1 second
                    ++iter;
                }
                if (MiscCommon::IsProcessExist(pidToKill))
                    LOG(MiscCommon::error) << "FAILED to close the process.";
            }
        }

      private:
        std::chrono::steady_clock::time_point m_startIdleTime;

        std::function<void(void)> m_idleCallback;
        std::vector<callbackFunction_t> m_registeredCallbackFunctions;

        std::mutex m_mutex; // Mutex for updateIdle call
    };
}

#endif /* defined(__DDS__MonitoringThread__) */
