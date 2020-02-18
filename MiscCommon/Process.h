// Copyright 2014 GSI, Inc. All rights reserved.
//
// This header contains a subset of helpers for Process, Daemon and PID file operations.
//
#ifndef PROCESS_H_
#define PROCESS_H_

// API
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wordexp.h>

#if defined(__APPLE__)
#include <sys/sysctl.h>
#endif

// STD
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
// POSIX regexp
#include <regex.h>
// MiscCommon
#include "CustomIterator.h"
#include "ErrorCode.h"
#include "MiscUtils.h"
#include "SysHelper.h"
#include "def.h"
#include "stlx.h"
// BOOST
#include <boost/asio.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#include <boost/process.hpp>
#pragma clang diagnostic pop

namespace bp = boost::process;
namespace bio = boost::asio;

namespace MiscCommon
{
    /**
     *
     * @brief The function checks, whether the process which corresponds to the given \b _PID can be found.
     * @param[in] _PID - a process ID to look for.
     * @return \b true when the process is found, otherwise return value is \b false.
     * @note This function will not be able to check existence of a zombie process
     *
     */
    inline bool IsProcessRunning(pid_t _PID)
    {
        // unlike ::kill we don't except negative or 0 pid.
        if (_PID <= 0)
            return false;

        return !(::kill(_PID, 0) == -1 && errno == ESRCH);
    }
    /**
     *
     * @brief A PID-file helper
     *
     */
    class CPIDFile
    {
      public:
        CPIDFile(const std::string& _FileName, pid_t _PID)
            : m_FileName(_FileName)
        {
            if (!_FileName.empty() && _PID > 0)
            {
                // Preventing to start a second "instance" if the pidfile references to the running process
                const pid_t pid = GetPIDFromFile(m_FileName);
                if (pid > 0 && IsProcessRunning(pid))
                {
                    // We don't want to unlink this file
                    m_FileName.clear();
                    throw std::runtime_error("Error creating pidfile. The process corresponding to pidfile \"" +
                                             _FileName + "\" is still running");
                }

                // Wrtiting new pidfile
                std::ofstream f(m_FileName.c_str());
                if (!f.is_open())
                    throw std::runtime_error("can't create PID file: " + m_FileName);

                f << _PID;
            }
            else
                m_FileName.clear();
        }

        ~CPIDFile()
        {
            if (!m_FileName.empty())
                ::unlink(m_FileName.c_str());
        }

        static pid_t GetPIDFromFile(const std::string& _FileName)
        {
            std::ifstream f(_FileName.c_str());
            if (!f.is_open())
                return 0;

            pid_t pid(0);
            f >> pid;

            return pid;
        }

      private:
        std::string m_FileName;
    };

#if defined(__APPLE__)
    /**
     *
     * @brief Getting List of All Processes on Mac OS X.
     * @note This class gets a list of all BSD processes, which includes
     * daemon processes, using the BSD sysctl routine.
     *
     */
    class CFindProcess
    {
      public:
        typedef std::set<pid_t> ProcContainer_t;

      public:
        static void getAllPIDsForProcessName(const std::string& _processName,
                                             ProcContainer_t* _pidContainer,
                                             bool _filterForRealUserID = false)
        {
            // Setting up the mib (Management Information Base)
            // We pass CTL_KERN, KERN_PROC, KERN_PROC_ALL to sysctl as the MIB
            // to get back a BSD structure with all BSD process information for
            // all processes in it (including BSD process names)
            int mib[3] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };

            size_t buffSize = 0; // set to zero to start with.
            int error = 0;
            struct kinfo_proc* BSDProcInfo = NULL;

            if (_processName.empty())
                return;
            if (!_pidContainer)
                return;

            _pidContainer->clear();

            // Getting list of process information for all processes
            bool done(false);
            do
            {
                error = sysctl(mib, 3, NULL, &buffSize, NULL, 0);
                if (error != 0)
                    throw system_error("Error occurred while retrieving a running processes list");

                BSDProcInfo = (struct kinfo_proc*)malloc(buffSize);

                if (BSDProcInfo == NULL)
                    throw system_error(
                        "Error occurred while retrieving a running processes list. Unable to allocate the buffer.");

                error = sysctl(mib, 3, BSDProcInfo, &buffSize, NULL, 0);

                // Here we successfully got the process information.
                // Thus set the variable to end this sysctl calling loop
                if (error == 0)
                {
                    done = true;
                }
                else
                {
                    // failed getting process information we will try again next time around the loop.  Note this is
                    // caused
                    // by the fact the process list changed between getting the size of the buffer and actually filling
                    // the buffer (something which will happen from time to time since the process list is dynamic).
                    // Anyways, the attempted sysctl call failed.  We will now begin again by freeing up the allocated
                    // buffer and starting again at the beginning of the loop.
                    free(BSDProcInfo);
                }
            } while (!done);

            // Going through process list looking for processes with matching names

            uid_t userid = getuid();

            const size_t processCount = buffSize / sizeof(struct kinfo_proc);
            for (size_t i = 0; i < processCount; ++i)
            {
                // Getting PID of process we are examining
                const pid_t pid = BSDProcInfo[i].kp_proc.p_pid;
                // Getting name of process we are examining
                const std::string name = BSDProcInfo[i].kp_proc.p_comm;
                // Getting real user if of the process
                const uid_t uid = BSDProcInfo[i].kp_eproc.e_pcred.p_ruid;

                if ((pid > 0) && (name == _processName))
                {
                    if (!_filterForRealUserID)
                        _pidContainer->insert(pid);
                    else if (uid == userid)
                        _pidContainer->insert(pid);
                }
            }

            free(BSDProcInfo);
        }

        static bool pidExists(int _pid)
        {
            int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
            size_t length = 0;
            if (sysctl(name, 4, NULL, &length, NULL, 0))
                throw system_error("Unable to call sysctl in the pidExists function.");
            kinfo_proc* result = (kinfo_proc*)malloc(length);
            if (sysctl(name, 4, result, &length, NULL, 0))
                throw system_error("Unable to call sysctl in the pidExists function.");
            int i, procCount = length / sizeof(kinfo_proc);
            for (i = 0; i < procCount; ++i)
            {
                kinfo_proc* test = &result[i];

                if (test->kp_proc.p_pid == _pid)
                {
                    free(result);
                    return true;
                }
                test = NULL;
            }

            free(result);
            return false;
        }
    };
#endif

/**
 *
 * @brief This class is used to quarry a list of currently running processes
 * @note The class is using proc filesystem (Linux)
 * @note Usage: In the example container pids will be containing pids of currently running processes
 * @code
 CProcList::ProcContainer_t pids;
 CProcList::GetProcList( &pids );
 * @endcode
 *
 */
#if !defined(__APPLE__)
    class CProcList
    {
      public:
        typedef std::set<pid_t> ProcContainer_t;

      public:
        static void GetProcList(ProcContainer_t* _Procs)
        {
            if (!_Procs)
                throw std::invalid_argument("CProcList::GetProcList: Input container is NULL");

            _Procs->clear();

            struct dirent** namelist;
            // scanning the "/proc" filesystem
            int n = scandir("/proc", &namelist, CheckDigit, alphasort);

            if (-1 == n)
                throw system_error("CProcList::GetProcList exception");
            if (0 == n)
                return; // there were no files

            for (int i = 0; i < n; ++i)
            {
                std::stringstream ss(namelist[i]->d_name);
                pid_t pid;
                ss >> pid;
                _Procs->insert(pid);
                free(namelist[i]);
            }

            free(namelist);
        }

      private:
        static int CheckDigit(const struct dirent* _d)
        {
            const std::string sName(_d->d_name);
            // Checking whether file name has all digits
            return (sName.end() == std::find_if(sName.begin(), sName.end(), std::not1(IsDigit())));
        }
    };
#endif

#if !defined(__APPLE__)
    /**
     *
     * @brief This class helps to retrieve process's information from /proc/\<pid\>/status
     * @note Usage:
     * @code
     CProcStatus p;
     p.Open( 8007 );
     cout << "Name" << p.GetValue( "Name" ) << endl;
     cout << "PPid" << p.GetValue( "PPid" ) << endl;
     p.Open( 1 );
     cout << "Name" << p.GetValue( "Name" ) << endl;
     cout << "PPid" << p.GetValue( "PPid" ) << endl;
     * @endcode
     *
     */
    // TODO: need a new algorithms for a longer app names retrieval
    class CProcStatus
    {
        typedef std::shared_ptr<std::ifstream> ifstream_ptr;
        typedef std::map<std::string, std::string> keyvalue_t;

      public:
        CProcStatus()
        {
            // Preparing regular expression pattern
            regcomp(&m_re, "(.*):(.*)", REG_EXTENDED);
        }
        ~CProcStatus()
        {
            regfree(&m_re);
        }
        void Open(pid_t _PId)
        {
            m_values.clear();
            if (m_f.get())
                m_f->close();

            std::stringstream ss;
            ss << "/proc/" << _PId << "/status";
            m_f = ifstream_ptr(new std::ifstream(ss.str().c_str()));
            // create reader objects
            // HACK: the extra set of parenthesis (the last argument of vector's ctor) is required (for gcc 4.1+)
            //      StringVector_t vec( custom_istream_iterator<std::string>(*m_f),
            //      (custom_istream_iterator<std::string>()) );
            // or
            // custom_istream_iterator<std::string> in_begin(*m_f);
            //      custom_istream_iterator<std::string> in_end;
            //      StringVector_t vec( in_begin, in_end );
            // the last method for gcc 3.2+
            // , because
            // the compiler is very aggressive in identifying function declarations and will identify the
            // definition of vec as forward declaration of a function accepting two istream_iterator parameters
            // and returning a vector of integers
            custom_istream_iterator<std::string> in_begin(*m_f);
            custom_istream_iterator<std::string> in_end;
            StringVector_t vec(in_begin, in_end);

            for_each(vec.begin(), vec.end(), std::bind1st(MiscCommon::stlx::mem_fun(&CProcStatus::_Parser), this));
        }
        std::string GetValue(const std::string& _KeyName) const
        {
            // We want to be case insensitive
            std::string sKey(_KeyName);
            to_lower(sKey);

            keyvalue_t::const_iterator iter = m_values.find(sKey);
            return (m_values.end() == iter ? std::string() : iter->second);
        }

      private:
        bool _Parser(const std::string& _sVal)
        {
            regmatch_t PMatch[3];
            if (0 != regexec(&m_re, _sVal.c_str(), 3, PMatch, 0))
                return false;
            std::string sKey(_sVal.c_str() + PMatch[1].rm_so, PMatch[1].rm_eo - PMatch[1].rm_so);
            std::string sValue(_sVal.c_str() + PMatch[2].rm_so, PMatch[2].rm_eo - PMatch[2].rm_so);
            // We want to be case insensitive
            to_lower(sKey);

            trim<std::string>(&sValue, '\t');
            trim<std::string>(&sValue, ' ');
            // insert key-value if found
            m_values.insert(keyvalue_t::value_type(sKey, sValue));
            return true;
        }

      private:
        ifstream_ptr m_f;
        regex_t m_re;
        keyvalue_t m_values;
    };
#endif
/**
 *
 *
 */
#if !defined(__APPLE__)
    struct SFindName : public std::binary_function<CProcList::ProcContainer_t::value_type, std::string, bool>
    {
        bool operator()(CProcList::ProcContainer_t::value_type _pid, const std::string& _Name) const
        {
            CProcStatus p;
            p.Open(_pid);
            return (p.GetValue("Name") == _Name);
        }
    };
#endif
    /**
     *
     *
     */
    typedef std::vector<pid_t> vectorPid_t;

    inline vectorPid_t getprocbyname(const std::string& _Srv, bool _filterForRealUserID = false)
    {
        vectorPid_t retVal;
#if defined(__APPLE__)
        CFindProcess::ProcContainer_t container;
        CFindProcess::getAllPIDsForProcessName(_Srv, &container, _filterForRealUserID);
        copy(container.begin(), container.end(), std::back_inserter(retVal));
#else
        CProcList::ProcContainer_t pids;
        CProcList::GetProcList(&pids);

        CProcList::ProcContainer_t::const_iterator iter = pids.begin();
        while (true)
        {
            iter = std::find_if(iter, pids.end(), std::bind2nd(SFindName(), _Srv));
            if (pids.end() == iter)
                break;

            retVal.push_back(*iter);
            ++iter;
        };
#endif
        return retVal;
    }

    /**
     *
     *
     */
    inline bool is_status_ok(int status)
    {
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }

    /**
     *
     *
     */
    inline pid_t execute(const std::string& _Command,
                         const std::string& _stdoutFileName,
                         const std::string& _stderrFileName)
    {
        try
        {
            std::string smartCmd(_Command);
            MiscCommon::smart_path(&smartCmd);

            // Restore default handler. If we fail to do so, we might fail to waitpid our children.
            // After we started using boost::process we noticed that ::waitpid fails.
            // boost:process either sets its own handler or there is a call for signal(SIGCHLD, SIG_IGN);
            signal(SIGCHLD, SIG_DFL);
            // Execute the process
            bp::child c(smartCmd, bp::std_out > _stdoutFileName, bp::std_err > _stderrFileName);
            pid_t pid = c.id();
            c.detach();
            return pid;
        }
        catch (std::exception& _e)
        {
            std::stringstream ss;
            ss << "execute: " << _e.what();
            throw std::runtime_error(ss.str());
        }
    }

    /// We do not want the parent process to wait for its child process and we do not want to create a zombie.
    inline void execute(const std::string& _Command)
    {
        try
        {
            std::string smartCmd(_Command);
            MiscCommon::smart_path(&smartCmd);
            bp::spawn(smartCmd);
        }
        catch (std::exception& _e)
        {
            std::stringstream ss;
            ss << "execute: " << _e.what();
            throw std::runtime_error(ss.str());
        }
    }

    // TODO: Document me!
    // If _Timeout is 0, then function returns child pid and doesn't wait for the child process to finish. Otherwise
    // return value is 0.
    inline pid_t execute(
        const std::string& _Command,
        const std::chrono::seconds& _Timeout,
        std::string* _output = nullptr,
        std::string* _errout = nullptr,
        int* _exitCode = nullptr,
        const std::chrono::seconds& _unit_test_Sleep = std::chrono::seconds(0) /*used for unit tests only*/)
    {
        try
        {
            std::string smartCmd(_Command);
            MiscCommon::smart_path(&smartCmd);

            if (std::chrono::seconds(0) == _Timeout)
            {
                boost::process::child c(smartCmd);
                pid_t pid = c.id();
                c.detach();
                return pid;
            }

            bio::io_context ios;
            std::future<std::string> out_data;
            std::future<std::string> err_data;

            bp::group g;
            bp::child c(smartCmd, g, bp::std_in.close(), bp::std_out > out_data, bp::std_err > err_data, ios);

            if (!c.valid())
                throw std::runtime_error("Can't execute the given process.");

            if (_unit_test_Sleep > std::chrono::seconds(0))
                std::this_thread::sleep_for(_unit_test_Sleep);

            bool errorFlag(false);
            // A watchdog thread for this process
            std::thread watchdogThread{ [&]() {
                std::future_status status = out_data.wait_for(_Timeout);
                if (status == std::future_status::deferred)
                {
                    throw std::runtime_error("Can't execute the process: future status is deferred.");
                }
                else if (status == std::future_status::ready)
                {
                }
                else if (status == std::future_status::timeout)
                {
                    // Child didn't yet finish. Terminating it...
                    g.terminate();
                    ios.stop();
                    errorFlag = true;
                }
            } };

            ios.run();
            watchdogThread.join();

            // prevent leaving a zombie process
            c.wait();

            if (errorFlag)
                throw std::runtime_error("Timeout has been reached, command execution will be terminated.");

            if (_output)
                *_output = out_data.get();

            if (_errout)
                *_errout = err_data.get();

            if (_exitCode)
                *_exitCode = c.exit_code();

            return 0;
        }
        catch (std::exception& _e)
        {
            std::stringstream ss;
            ss << "execute: " << _e.what();
            throw std::runtime_error(ss.str());
        }
    }
}; // namespace MiscCommon

#endif /*PROCESS_H_*/
