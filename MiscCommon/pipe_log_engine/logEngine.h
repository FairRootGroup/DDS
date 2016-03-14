// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef LOGENGINE_H
#define LOGENGINE_H
//=============================================================================
#include <boost/thread/thread.hpp>
#include <csignal>
#include <string>
//=============================================================================
namespace dds
{
    namespace pipe_log_engine
    {
        class CLogEngine
        {
          public:
            typedef std::function<void(const std::string&)> onLogEvent_t;

          public:
            CLogEngine(bool _debugMode = false);
            ~CLogEngine();

            void start(const std::string& _pipeFilePath, onLogEvent_t _callback = nullptr);
            void stop();
            void operator()(const std::string& _msg, const std::string& _id = "**", bool _debugMsg = false) const;
            void debug_msg(const std::string& _msg, const std::string& _id = "**") const
            {
                operator()(_msg, _id, true);
            }
            void setDbgFlag(bool _dbgFlag)
            {
                m_debugMode = _dbgFlag;
            }

          private:
            void thread_worker(int _fd, const std::string& _pipename);

          private:
            int m_fd;
            boost::thread* m_thread;
            std::string m_pipeName;
            bool m_debugMode;
            volatile sig_atomic_t m_stopLogEngine;
            onLogEvent_t m_callback;
        };
    }
}
//=============================================================================
#endif
