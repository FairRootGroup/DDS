// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "MonitoringThread.h"
#include "Logger.h"
#include "Process.h"
#include "UserDefaults.h"
// STL
#include <thread>
#include <mutex>
// API
#include <csignal>

using namespace std;
using namespace dds;
using namespace MiscCommon;

CMonitoringThread::CMonitoringThread()
{
}

CMonitoringThread::~CMonitoringThread()
{
}

CMonitoringThread& CMonitoringThread::instance()
{
    static CMonitoringThread instance;
    return instance;
}

void CMonitoringThread::start(const std::function<void(void)>& _idleCallback)
{
    static const float LOOP_TIME_DELAY = 5.f;
    static const float IDLE_TIME = 30.f;
    static const float WAITING_TIME = 20.f;

    m_startTime = chrono::steady_clock::now();
    m_startIdleTime = chrono::steady_clock::now();

    thread t([this, &_idleCallback]()
             {
                 while (true)
                 {
                     chrono::steady_clock::time_point currentTime = chrono::steady_clock::now();
                     chrono::duration<double> elapsedTime =
                         chrono::duration_cast<chrono::duration<double>>(currentTime - m_startTime);
                     LOG(MiscCommon::info) << "time since start [s]: " << elapsedTime.count();

                     // Check if process is idle.
                     // First try to call callback -> wait some time -> call terminate -> wait some time -> kill
                     // process.
                     chrono::duration<double> idleTime =
                         chrono::duration_cast<chrono::duration<double>>(currentTime - m_startTime);
                     if (idleTime.count() > IDLE_TIME)
                     {
                         // First call idle callback
                         LOG(info) << "Process is idle call idle callback";
                         _idleCallback();
                         sleep(WAITING_TIME);

                         // Call terminate
                         // LOG(error) << "Process is idle call terminate";
                         // terminate();
                         // sleep(WAITING_TIME);

                         // Kill process
                         LOG(error) << "Process is idle try to kill the process";
                         killProcess();
                     }

                     sleep(LOOP_TIME_DELAY);
                 }
             });
    t.detach();
}

void CMonitoringThread::updateIdle()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_startIdleTime = chrono::steady_clock::now();
}

void CMonitoringThread::killProcess()
{
    string pidFileName(CUserDefaults::getDDSPath());
    pidFileName += "dds-agent.pid";

    // TODO: make wait for the process here to check for errors
    const pid_t pidToKill = CPIDFile::GetPIDFromFile(pidFileName);
    if (pidToKill > 0 && IsProcessExist(pidToKill))
    {
        LOG(log_stdout) << PROJECT_NAME << ": self exiting (" << pidToKill << ")...";
        // TODO: Maybe we need more validations of the process before
        // sending a signal. We don't want to kill someone else.
        kill(pidToKill, SIGTERM);

        // Waiting for the process to finish
        size_t iter(0);
        const size_t max_iter = 30;
        while (iter <= max_iter)
        {
            if (!IsProcessExist(pidToKill))
            {
                cout << endl;
                break;
            }
            LOG(log_stdout) << ".";
            sleep(1); // sleeping for 1 second
            ++iter;
        }
        if (IsProcessExist(pidToKill))
            LOG(error) << "FAILED to close the process.";
    }
}
