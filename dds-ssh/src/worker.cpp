/*
 *  worker.cpp
 *  pod-ssh
 *
 *  Created by Anar Manafov on 16.06.10.
 *  Copyright 2010 Anar Manafov <Anar.Manafov@gmail.com>. All rights reserved.
 *
 */
//=============================================================================
// pod-ssh
#include "worker.h"
// MiscCommon
#include "SysHelper.h"
#include "Process.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
const size_t g_cmdTimeout = 35; // in sec.
//=============================================================================
CWorker::CWorker(configRecord_t _rec, log_func_t* _log, const SWNOptions& _options)
    : m_rec(_rec)
    , m_log(_log)
    , m_options(_options)
    , m_mutex(mutexPtr_t(new boost::mutex()))
{
    // constructing a full path of the worker for this id
    // pattern: <m_wrkDir>/<m_id>
    smart_append(&m_rec->m_wrkDir, '/');
    smart_path(&m_rec->m_wrkDir);
    m_rec->m_wrkDir += m_rec->m_id;
}
//=============================================================================
CWorker::~CWorker()
{
}
//=============================================================================
void CWorker::printInfo(ostream& _stream) const
{
    _stream << "[" << m_rec->m_id << "] with ";
    if (0 == m_rec->m_nWorkers)
        _stream << "a dynamic number of";
    else
        _stream << m_rec->m_nWorkers;

    _stream << " workers at " << m_rec->m_addr << ":" << m_rec->m_wrkDir;
}
//=============================================================================
bool CWorker::runTask(ETaskType _param) const
{
    // protection: don't execute different tasks on the same worker in the same time
    boost::mutex::scoped_lock lck(*m_mutex);

    StringVector_t params;
    params.push_back("-i" + m_rec->m_id);
    params.push_back("-l" + m_rec->m_addr);
    params.push_back("-w" + m_rec->m_wrkDir);
    if (!m_rec->m_sshOptions.empty())
        params.push_back("-o" + m_rec->m_sshOptions);
    if (m_options.m_debug)
        params.push_back("-d");

    string cmd;
    switch (_param)
    {
        case task_submit:
        {
            stringstream ss;
            ss << "-n" << m_rec->m_nWorkers;
            params.push_back(ss.str());
            cmd = "$POD_LOCATION/bin/private/pod-ssh-submit-worker";
            break;
        }
        case task_clean:
        {
            if (m_options.m_logs)
                params.push_back("-m");
            if (m_options.m_fastClean)
                params.push_back("-f");

            cmd = "$POD_LOCATION/bin/private/pod-ssh-clean-worker";
            break;
        }
        case task_status:
            cmd = "$POD_LOCATION/bin/private/pod-ssh-status-worker";
            break;
        case task_exec:
            params.push_back("-e " + m_options.m_scriptName);
            cmd = "$POD_LOCATION/bin/private/pod-ssh-exec-worker";
            break;
    }

    smart_path(&cmd);
    return exec_command(cmd, params);
}
//=============================================================================
bool CWorker::exec_command(const string& _cmd, const StringVector_t& _params) const
{
    string outPut;
    try
    {
        do_execv(_cmd, _params, g_cmdTimeout, &outPut);
    }
    catch (exception& e)
    {
        log("Failed to process the task.\n");
        return false;
    }
    if (!outPut.empty())
    {
        ostringstream ss;
        ss << "Cmnd Output: " << outPut << "\n";
        log(ss.str());
    }
    return true;
}
//=============================================================================
void CWorker::log(const std::string& _msg) const
{
    (*m_log)(_msg, m_rec->m_id, true);
}
