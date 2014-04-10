// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
//=============================================================================
#include "logEngine.h"
// BOOST
#include <boost/bind.hpp>
// MiscCommon
#include "SysHelper.h"
// API
#include <limits.h> // for PIPE_BUF
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
CLogEngine::~CLogEngine()
{
    stop();
}
//=============================================================================
void CLogEngine::start(const string& _pipeFilePath)
{
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
            throw MiscCommon::system_error("LogEngine: Write error");
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
            throw MiscCommon::system_error("LogEngine: Write error");
        total += n;
    }
}
//=============================================================================
void CLogEngine::thread_worker(int _fd, const string& _pipename)
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
            cerr << "Problem in the log engine: " << errno2str() << endl;
            break;
        }

        if (FD_ISSET(_fd, &readset))
        {
            const int read_size = 64;
            char buf[read_size];
            int numread(0);
            while (true)
            {
                numread = read(_fd, buf, read_size);
                // don't print the last Control character
                // it was sent just to wake up the thread
                if (m_stopLogEngine && 1 == numread)
                    break;
                if (numread > 0)
                    cout << string(buf, numread);
                else
                    break;
            }
            cout.flush();
        }
    }
}
