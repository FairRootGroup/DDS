// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
//=============================================================================
#include "logEngine.h"
// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/bind/bind.hpp>
// MiscCommon
#include "Logger.h"
#include "SysHelper.h"
// API
#include <limits.h> // for PIPE_BUF
//=============================================================================
using namespace std;
using namespace dds;
using namespace dds::pipe_log_engine;
using namespace MiscCommon;
//=============================================================================
CLogEngine::CLogEngine(bool _debugMode)
    : m_fd(0)
    , m_thread(NULL)
    , m_debugMode(_debugMode)
    , m_stopLogEngine(0)
{
    Logger::instance().init(); // Initialize log
}
//=============================================================================
CLogEngine::~CLogEngine()
{
    stop();
}
//=============================================================================
void CLogEngine::start(const string& _pipeFilePath, onLogEvent_t _callback)
{
    m_callback = _callback;
    m_stopLogEngine = 0;
    // create a named pipe
    // it's used to collect outputs from the threads and called shell scripts...
    m_pipeName = _pipeFilePath;
    smart_path(&m_pipeName);
    int ret_val = mkfifo(m_pipeName.c_str(), 0666);
    if ((-1 == ret_val) && (EEXIST != errno))
        throw runtime_error("Can't create a named pipe: " + m_pipeName);

    // Open the pipe for reading
    m_fd = open(m_pipeName.c_str(), O_RDWR | O_NONBLOCK);
    if ((-1 == m_fd) && (EEXIST != errno))
        throw runtime_error("Can't opem a named pipe: " + m_pipeName);

    // Start the log engine
    m_thread = new boost::thread(boost::bind(&CLogEngine::thread_worker, this, m_fd, m_pipeName));
    LOG(info) << "pipe log engine has been started: " << m_pipeName;
}
//=============================================================================
void CLogEngine::stop()
{
    if (NULL != m_thread)
    {
        m_stopLogEngine = 1;
        // send just *one* charecter to wake up the thread.
        this->operator()("\0", "");
        m_thread->join();
        delete m_thread;
        m_thread = NULL;
    }

    if (m_fd > 0)
    {
        close(m_fd);
        m_fd = 0;
    }

    unlink(m_pipeName.c_str());

    LOG(info) << "pipe log engine has been stopped: " << m_pipeName;
}
//=============================================================================
void CLogEngine::operator()(const string& _msg, const string& _id, bool _debugMsg) const
{
    if (!m_fd)
        return;

    if (_debugMsg && !m_debugMode)
        return;

    // this is the stop signal from the "stop" method
    if (_msg.empty() && _id.empty())
    {
        if (write(m_fd, "\0", 1) < 0)
            throw MiscCommon::system_error("pipe log engine: Write error");
        return;
    }

    // All the following calls must be thread-safe.

    string out(_id);
    char timestr[200];

    // print date/time only when printing debug messages
    if (m_debugMode)
    {
        // print time with RFC 2822 - compliant date format
        time_t t = time(NULL);
        struct tm tmp;
        if (localtime_r(&t, &tmp) == NULL)
        {
            // TODO: log it.
            return;
        }

        if (strftime(timestr, sizeof(timestr), "%a, %d %b %Y %T %z", &tmp) == 0)
        {
            // TODO: log it.
            return;
        }
    }

    // write to a pipe is an atomic operation,
    // according to POSIX we just need to be shorter than PIPE_BUF

    // print date/time only when printing debug messages
    if (m_debugMode)
    {
        out += "\t[";
        out += timestr;
        out += "]\t";
    }
    else
        out += "\t";

    out += _msg;

    // We need to send by PIPE_BUF portions
    CHARVector_t buf;
    buf.reserve(out.size());
    copy(out.begin(), out.end(), back_inserter(buf));

    size_t total = 0;
    int n = 0;
    size_t len = buf.size();
    while (total < len)
    {
        if ((n = write(m_fd, &buf[total], len - total)) < 0)
            throw MiscCommon::system_error("pipe log engine: Write error");
        total += n;
    }
}
//=============================================================================
void CLogEngine::logMsg(const string& _msg)
{
    string sMsg(_msg);
    // trim it
    boost::trim(sMsg);
    // remove new line before logging the message
    boost::trim_left_if(sMsg, boost::is_any_of("\r\n"));

    // do we need to do anything?
    if (sMsg.empty())
        return; // I guess not

    LOG(info) << sMsg;
    // call user's callback if needed
    if (m_callback != nullptr)
        m_callback(sMsg);
}
//=============================================================================
void CLogEngine::thread_worker(int _fd, const string& /*_pipename*/)
{
    while (_fd > 0 && !m_stopLogEngine)
    {
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(_fd, &readset);
        int retval = ::select(_fd + 1, &readset, NULL, NULL, NULL);

        if (EBADF == errno)
            break;

        if (retval < 0)
        {
            LOG(error) << "pipe log engine: "
                       << "Problem in the log engine: " << errno2str();
            break;
        }

        if (FD_ISSET(_fd, &readset))
        {
            const int read_size = 64;
            char buf[read_size];
            string input;
            while (true)
            {
                int numread = read(_fd, buf, read_size);
                // don't print the last Control character
                // it was sent just to wake up the thread
                if (m_stopLogEngine && 1 == numread)
                    break;
                if (numread > 0)
                {
                    input += string(buf, numread);

                    if (input.find('\n') != string::npos)
                    {
                        std::string full_line(input.begin(), std::find(input.begin(), input.end(), '\n'));
                        std::string rest_lines(std::find(input.begin(), input.end(), '\n'), input.end());
                        if (!full_line.empty())
                        {
                            input.clear();
                            // log the message
                            logMsg(full_line);
                        }
                        input = rest_lines;
                    }
                }
                else
                    break;
            }
            // log the rest of the line
            if (!input.empty())
                logMsg(input);
        }
    }
}
