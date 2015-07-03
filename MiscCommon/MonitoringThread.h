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
        // the function to call, the interval (in sec) the function should be called at
        typedef std::pair<callbackFunction_t, std::chrono::seconds> callbackValue_t;

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
            // Looping monitoring thread with a step of 1 sec up to *Unlimited* sec (size of int)
            static const std::chrono::seconds INTERVAL_STEP(1);
            static const std::chrono::seconds WAITING_TIME(20);

            m_startIdleTime = std::chrono::steady_clock::now();

            std::thread t(
                [this, &_idleCallback, _idleTime]()
                {
                    try
                    {
                        std::chrono::seconds secInterval(0);
                        while (true)
                        {
                            // handle exceptions of the custom actions separately to prevent breaks of the
                            // monitoring thread
                            try
                            {
                                std::lock_guard<std::mutex> lock(m_registeredCallbackFunctionsMutex);
                                // Call registered callback functions
                                // We use Erase-remove idiom to execute callback and remove expired if needed.
                                m_registeredCallbackFunctions.erase(
                                    remove_if(m_registeredCallbackFunctions.begin(),
                                              m_registeredCallbackFunctions.end(),
                                              [&](callbackValue_t& i)
                                              {
                                                  // A callback function can return
                                                  // false, which
                                                  // means it wants to be unregistered
                                                  // (expire)
                                                  const int nCurInterval =
                                                      std::chrono::duration<int>(secInterval).count();
                                                  const int nInterval = std::chrono::duration<int>(i.second).count();
                                                  if (nCurInterval != 0 && nCurInterval >= nInterval &&
                                                      0 == (nCurInterval % nInterval))
                                                  {
                                                      LOG(MiscCommon::debug)
                                                          << "MONITORING: calling callback at interval of "
                                                          << std::chrono::duration<int>(i.second).count();
                                                      return (!i.first());
                                                  }
                                                  return false;
                                              }),
                                    m_registeredCallbackFunctions.end());
                            }
                            catch (std::exception& _e)
                            {
                                LOG(MiscCommon::error) << "MonitoringThread exception on custom actions: " << _e.what();
                            }
                            catch (...)
                            {
                                // Ignore any exception here to let the monitoring thread continue whatever it takes
                            }

                            std::chrono::seconds idleTime;
                            // Check if process is idle.
                            {
                                std::lock_guard<std::mutex> lock(m_mutex);
                                std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
                                idleTime =
                                    std::chrono::duration_cast<std::chrono::seconds>(currentTime - m_startIdleTime);
                            }

                            if (idleTime.count() > _idleTime)
                            {
                                // First call idle callback
                                LOG(MiscCommon::info) << "The process is idle for " << idleTime.count()
                                                      << " sec. Call idle callback and wait "
                                                      << std::chrono::duration<int>(WAITING_TIME).count() << "s";
                                // handle exceptions of the custom idle callback separately to prevent breaks of the
                                // monitoring thread
                                try
                                {
                                    _idleCallback();
                                }
                                catch (std::exception& _e)
                                {
                                    LOG(MiscCommon::error) << "MonitoringThread exception on custom idle function: "
                                                           << _e.what();
                                }
                                catch (...)
                                {
                                    // Ignore any exception here to let the monitoring thread continue whatever it takes
                                }
                                std::this_thread::sleep_for(WAITING_TIME);

                                // Call terminate
                                LOG(MiscCommon::info) << "Sending SIGTERM to this process...";
                                std::raise(SIGTERM);
                                std::this_thread::sleep_for(WAITING_TIME);

                                // Kill process
                                LOG(MiscCommon::info) << "The process still exists. Killing the process...";
                                killProcess();
                            }

                            std::this_thread::sleep_for(INTERVAL_STEP);
                            secInterval += INTERVAL_STEP;
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

        void registerCallbackFunction(callbackFunction_t _handler, const std::chrono::seconds& _interval)
        {
            std::lock_guard<std::mutex> lock(m_registeredCallbackFunctionsMutex);
            m_registeredCallbackFunctions.push_back(make_pair(_handler, _interval));
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
        std::vector<callbackValue_t> m_registeredCallbackFunctions;

        std::mutex m_registeredCallbackFunctionsMutex;

        std::mutex m_mutex; // Mutex for updateIdle call
    };
}

#endif /* defined(__DDS__MonitoringThread__) */
