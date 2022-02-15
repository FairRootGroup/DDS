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
using namespace dds::misc;
;
using namespace dds::user_defaults_api;
namespace fs = boost::filesystem;
//=============================================================================
const std::chrono::seconds g_cmdTimeout = std::chrono::seconds(20);
//=============================================================================
CWorker::CWorker(dds::configRecord_t _rec, const SWNOptions& _options, const std::string& _path)
    : m_rec(_rec)
    , m_options(_options)
    , m_path(_path)
{
    // constructing a full path of the worker for this id
    // pattern: <m_wrkDir>/<sessionID>/<m_id>
    smart_path(&m_rec->m_wrkDir);
    fs::path pathWrk(m_rec->m_wrkDir);
    pathWrk /= CUserDefaults::instance().getCurrentSID();
    pathWrk /= m_rec->m_id;

    m_rec->m_wrkDir = pathWrk.string();
}
//=============================================================================
CWorker::~CWorker()
{
}
//=============================================================================
void CWorker::printInfo(ostream& _stream) const
{
    _stream << "[" << m_rec->m_id << "] with " << m_rec->m_nSlots << " slots at " << m_rec->m_addr << ":"
            << m_rec->m_wrkDir;
}
//=============================================================================
bool CWorker::run(ETaskType _param)
{
    stringstream ssParams;
    ssParams << " -i \"" << m_rec->m_id << "\" -l \"" << m_rec->m_addr << "\" -w \"" << m_rec->m_wrkDir << "\" -s \""
             << m_rec->m_submissionID << "\" -a \"" << CUserDefaults::instance().getCurrentSID() << "\"";
    if (!m_rec->m_sshOptions.empty())
        ssParams << " -o \"" << m_rec->m_sshOptions << "\"";

    stringstream ssCmd;
    switch (_param)
    {
        case task_submit:
        {
            ssParams << " -n \"" << m_rec->m_nSlots << "\"";
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
    if (_cmd.empty())
        return false;

    log(_cmd);
    string outPut;
    int nExitCode(0);
    try
    {
        string stdout;
        string stderr;
        LOG(info) << "Executing task [" << m_rec->m_id << "] " << _cmd;

        execute(_cmd, g_cmdTimeout, &stdout, &stderr, &nExitCode);

        if (!stdout.empty())
            LOG(info) << "Task outoput for [" << m_rec->m_id << "] " << stdout;

        if (!stderr.empty())
            LOG(error) << "Task outoput for [" << m_rec->m_id << "] " << stderr;

        LOG(info) << "Task execution [" << m_rec->m_id << "] finished with code: " << nExitCode;

        if (nExitCode != 0)
            return false;
    }
    catch (exception& e)
    {
        LOG(error) << "Failed to process the task [" << m_rec->m_id << "]: " << e.what();
        return false;
    }

    return true;
}
//=============================================================================
void CWorker::log(const std::string& _msg) const
{
    LOG(debug) << "[" << m_rec->m_id << "]" << _msg;
}
