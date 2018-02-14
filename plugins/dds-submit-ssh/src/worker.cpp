// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
//=============================================================================
// pod-ssh
#include "worker.h"
// MiscCommon
#include "Logger.h"
#include "Process.h"
#include "SysHelper.h"
//=============================================================================
using namespace std;
using namespace dds;
using namespace dds::ssh_cmd;
using namespace MiscCommon;
//=============================================================================
const std::chrono::seconds g_cmdTimeout = std::chrono::seconds(20);
//=============================================================================
CWorker::CWorker(ncf::configRecord_t _rec, const SWNOptions& _options, const string& _path)
    : m_rec(_rec)
    , m_options(_options)
    , m_path(_path)
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

    stringstream ssParams;
    ssParams << " -i " << m_rec->m_id << " -l " << m_rec->m_addr << " -w " << m_rec->m_wrkDir;
    if (!m_rec->m_sshOptions.empty())
        ssParams << " -o " << m_rec->m_sshOptions;

    stringstream ssCmd;
    switch (_param)
    {
        case task_submit:
        {
            ssParams << " -n " << m_rec->m_nWorkers;
            string cmd(m_path);
            cmd += "dds-submit-ssh-worker";
            smart_path(&cmd);
            ssCmd << cmd << " " << ssParams.str();
            break;
        }
        case task_clean:
        {
            //            if (m_options.m_logs)
            //                params.push_back("-m");
            //            if (m_options.m_fastClean)
            //                params.push_back("-f");
            //
            //            cmd = "$DDS_LOCATION/bin/private/dds-ssh-clean-worker";
            break;
        }
        case task_status:
            //            cmd = "$DDS_LOCATION/bin/private/dds-ssh-status-worker";
            break;
        case task_exec:
            //            params.push_back("-e " + m_options.m_scriptName);
            //            cmd = "$DDS_LOCATION/bin/private/dds-ssh-exec-worker";
            break;
    }

    return exec_command(ssCmd.str());
}
//=============================================================================
bool CWorker::exec_command(const string& _cmd) const
{
    string outPut;
    try
    {
        execute(_cmd, g_cmdTimeout, &outPut);
    }
    catch (exception& e)
    {
        log(string("Failed to process the task: ") + e.what());
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
    LOG(debug) << "[" << m_rec->m_id << "]" << _msg;
}
