// Copyright 2014 GSI, Inc. All rights reserved.
//
// Log engine core implementation.
//
#ifndef CLOGIMP_H
#define CLOGIMP_H

// STD
#include <stdexcept>
#include <memory>
// MiscCommon
#include "Log.h"

namespace MiscCommon
{
/**
 *
 * @brief It is a supporting macro, which declares GetModuleName method. Needed by MiscCommon::CLogImp.
 * @brief Must be declared in a child class of MiscCommon::CLogImp.
 *
 */
#define REGISTER_LOG_MODULE(name)     \
    std::string GetModuleName() const \
    {                                 \
        return name;                  \
    }

    /**
     *
     * @brief It represents logbook as a singleton.
     * @brief ofstream specialization of CLog
     *
     */
    class CLogSingleton
    {
        typedef std::auto_ptr<CFileLog> CFileLogPtr;

        CLogSingleton()
        {
        }
        ~CLogSingleton()
        {
        }

      public:
        int Init(const std::string& _LogFileName,
                 bool _CreateNew = false,
                 unsigned char _logLevel = LOG_SEVERITY_INFO | LOG_SEVERITY_WARNING | LOG_SEVERITY_FAULT |
                                           LOG_SEVERITY_CRITICAL_ERROR)
        {
            if (m_log.get())
                throw std::logic_error("Log's singleton class has been already initialized.");

            m_log = CFileLogPtr(new CFileLog(_LogFileName, _CreateNew, _logLevel));
            push(LOG_SEVERITY_INFO, 0, "LOG singleton", "LOG singleton has been initialized.");
            return 0;
        }
        static CLogSingleton& Instance()
        {
            static CLogSingleton log;
            return log;
        }
        void push(LOG_SEVERITY _Severity,
                  unsigned long _ErrorCode,
                  const std::string& _Module,
                  const std::string& _Message)
        {
            if (!m_log.get())
            {
                std::cerr << _Message << std::endl;
                return;
            }
            m_log->push(_Severity, _ErrorCode, _Module, _Message);
        }
        bool IsReady()
        {
            return (NULL != m_log.get());
        }

      private:
        CFileLogPtr m_log;
    };

    /**
     *
     * @brief Template class. High-end helper implementation of CLog, its ofstream specialization.
     * @note: a REGISTER_LOG_MODULE(module name) must be be declared in a child class body.
     * an example:
     * @code
     * class CFoo: public MiscCommon::CLogImp<CAgentServer>
     * {
     *   public:
     *          CFoo();
     *          ~CFoo();
     *          REGISTER_LOG_MODULE( Foo );
     * };
     * @endcode
     *
     */
    template <typename _T>
    class CLogImp
    {
      public:
        CLogImp()
        {
            if (CLogSingleton::Instance().IsReady())
                CLogSingleton::Instance().push(
                    LOG_SEVERITY_INFO, 0, g_cszMODULENAME_CORE, "Bringing >>> " + GetModuleName() + " <<< to life...");
        }
        ~CLogImp()
        {
            CLogSingleton::Instance().push(
                LOG_SEVERITY_INFO, 0, g_cszMODULENAME_CORE, "Shutting down >>> " + GetModuleName() + " <<<");
        }
        void InfoLog(const std::string& _Message)
        {
            return CLogSingleton::Instance().push(LOG_SEVERITY_INFO, 0, GetModuleName(), _Message);
        }
        void InfoLog(unsigned long _ErrorCode, const std::string& _Message)
        {
            return CLogSingleton::Instance().push(LOG_SEVERITY_INFO, _ErrorCode, GetModuleName(), _Message);
        }
        void WarningLog(unsigned long _ErrorCode, const std::string& _Message)
        {
            return CLogSingleton::Instance().push(LOG_SEVERITY_WARNING, _ErrorCode, GetModuleName(), _Message);
        }
        void FaultLog(unsigned long _ErrorCode, const std::string& _Message)
        {
            return CLogSingleton::Instance().push(LOG_SEVERITY_FAULT, _ErrorCode, GetModuleName(), _Message);
        }
        void CriticalErrLog(unsigned long _ErrorCode, const std::string& _Message)
        {
            return CLogSingleton::Instance().push(LOG_SEVERITY_CRITICAL_ERROR, _ErrorCode, GetModuleName(), _Message);
        }
        void DebugLog(unsigned long _ErrorCode, const std::string& _Message)
        {
            return CLogSingleton::Instance().push(LOG_SEVERITY_DEBUG, _ErrorCode, GetModuleName(), _Message);
        }
        void msgPush(LOG_SEVERITY _Severity, const std::string& _Message, unsigned long _ErrorCode = 0)
        {
            return CLogSingleton::Instance().push(_Severity, _ErrorCode, GetModuleName(), _Message);
        }

      private:
        std::string GetModuleName()
        {
            _T* pT = reinterpret_cast<_T*>(this);
            return pT->GetModuleName();
        }
    };
};
#endif
