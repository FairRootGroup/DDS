// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef LOGENGINE_H
#define LOGENGINE_H
//=============================================================================
#include <boost/thread/thread.hpp>
#include <csignal>
//=============================================================================
namespace dds
{
    namespace pipe_log_engine
    {
        class CLogEngine
        {
          public:
            CLogEngine(bool _debugMode = false)
                : m_fd(0)
                , m_thread(NULL)
                , m_debugMode(_debugMode)
                , m_stopLogEngine(0)
            {
            }
            ~CLogEngine();
            void start(const std::string& _pipeFilePath);
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
        };
    }
}
//=============================================================================
#endif
