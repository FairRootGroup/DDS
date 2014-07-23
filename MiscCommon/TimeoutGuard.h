// Copyright 2014 GSI, Inc. All rights reserved.
//
// The header declares a CTimeoutGuard class.
//
#ifndef TIMEOUTGUARD_H_
#define TIMEOUTGUARD_H_

// MiscCommon
#include "Process.h"
#include "BOOSTHelper.h"

namespace MiscCommon
{
    /**
     * @brief The class, which watches the running time of the process and sends SEGTERM when defined time-out is
     *reached.
     **/
    class CTimeoutGuard
    {
        CTimeoutGuard()
            : m_IsInit(false)
            , m_pid(0)
            , m_secTimeOut(0)
        {
        }
        ~CTimeoutGuard()
        {
        }

      public:
        void Init(pid_t _pid, size_t _timeout)
        {
            if (m_IsInit)
                throw std::logic_error("CTimeoutGuard is already initialized");

            m_pid = _pid;
            m_secTimeOut = _timeout;
            m_IsInit = true;
            m_Thread = MiscCommon::BOOSTHelper::Thread_PTR_t(
                new boost::thread(boost::bind(&CTimeoutGuard::ThreadWorker, this)));
        }
        static CTimeoutGuard& Instance()
        {
            static CTimeoutGuard obj;
            return obj;
        }
        void ThreadWorker() const
        {
            sleep(m_secTimeOut);
            if (m_pid > 0 && IsProcessExist(m_pid))
            {
                // TODO: log me!
                ::kill(m_pid, SIGTERM);
            }
        }

      private:
        bool m_IsInit;
        pid_t m_pid;
        size_t m_secTimeOut;
        MiscCommon::BOOSTHelper::Thread_PTR_t m_Thread;
    };
};
#endif /*TIMEOUTGUARD_H_*/
