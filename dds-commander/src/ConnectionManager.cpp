// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "ConnectionManager.h"
#include "ChannelId.h"
#include "CommandAttachmentImpl.h"
#include "Topology.h"
#include "ncf.h"
// BOOST
#include <boost/property_tree/json_parser.hpp>

// silence "Unused typedef" warning using clang 3.7+ and boost < 1.59
#if BOOST_VERSION < 105900
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#include <boost/property_tree/ini_parser.hpp>
#if BOOST_VERSION < 105900
#pragma clang diagnostic pop
#endif
#include <boost/filesystem.hpp>

// STD
#include <mutex>

using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::topology_api;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using namespace dds::ncf;
using namespace dds::intercom_api;
using namespace std;
using namespace MiscCommon;
namespace fs = boost::filesystem;

CConnectionManager::CConnectionManager(const SOptions_t& _options)
    : CConnectionManagerImpl<CAgentChannel, CConnectionManager>(20000, 22000, true)
    , m_statEnabled(false)
{
    LOG(info) << "CConnectionManager constructor";
}

CConnectionManager::~CConnectionManager()
{
}

void CConnectionManager::_start()
{
    auto self(this->shared_from_this());

    // Check RMS plug-in activity
    CMonitoringThread::instance().registerCallbackFunction(
        [this, self]() -> bool {
            try
            {
                m_SubmitAgents.checkPluginFailedToStart();
            }
            catch (exception& _e)
            {
                LOG(error) << "RMS plug-in monitor: error: " << _e.what();
            }

            return true;
        },
        chrono::seconds(15));
}

void CConnectionManager::_stop()
{
}

void CConnectionManager::newClientCreated(CAgentChannel::connectionPtr_t _newClient)
{
    _newClient->setStatEnabled(m_statEnabled);

    // Subscribe on protocol messages
    function<bool(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment, CAgentChannel * _channel)> fGET_LOG = [this](
        SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdGET_LOG(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_LOG>(fGET_LOG);

    function<bool(SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment, CAgentChannel * _channel)>
        fBINARY_ATTACHMENT_RECEIVED = [this](SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
                                             CAgentChannel* _channel) -> bool {
        return this->on_cmdBINARY_ATTACHMENT_RECEIVED(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdBINARY_ATTACHMENT_RECEIVED>(fBINARY_ATTACHMENT_RECEIVED);

    function<bool(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment, CAgentChannel * _channel)>
        fGET_AGENTS_INFO =
            [this](SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdGET_AGENTS_INFO(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_AGENTS_INFO>(fGET_AGENTS_INFO);

    function<bool(SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment, CAgentChannel * _channel)> fSUBMIT = [this](
        SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdSUBMIT(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSUBMIT>(fSUBMIT);

    function<bool(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment, CAgentChannel * _channel)>
        fACTIVATE_AGENT =
            [this](SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdACTIVATE_AGENT(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdACTIVATE_AGENT>(fACTIVATE_AGENT);

    function<bool(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment, CAgentChannel * _channel)>
        fSTOP_USER_TASK =
            [this](SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdSTOP_USER_TASK(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSTOP_USER_TASK>(fSTOP_USER_TASK);

    function<bool(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment, CAgentChannel * _channel)>
        fTRANSPORT_TEST =
            [this](SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdTRANSPORT_TEST(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdTRANSPORT_TEST>(fTRANSPORT_TEST);

    function<bool(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, CAgentChannel * _channel)> fSIMPLE_MSG =
        [this](SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdSIMPLE_MSG(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSIMPLE_MSG>(fSIMPLE_MSG);

    function<bool(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, CAgentChannel * _channel)> fUPDATE_KEY =
        [this](SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdUPDATE_KEY(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdUPDATE_KEY>(fUPDATE_KEY);

    function<bool(SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment, CAgentChannel * _channel)>
        fUSER_TASK_DONE =
            [this](SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdUSER_TASK_DONE(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdUSER_TASK_DONE>(fUSER_TASK_DONE);

    function<bool(SCommandAttachmentImpl<cmdGET_PROP_LIST>::ptr_t _attachment, CAgentChannel * _channel)>
        fGET_PROP_LIST =
            [this](SCommandAttachmentImpl<cmdGET_PROP_LIST>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdGET_PROP_LIST(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_PROP_LIST>(fGET_PROP_LIST);

    function<bool(SCommandAttachmentImpl<cmdGET_PROP_VALUES>::ptr_t _attachment, CAgentChannel * _channel)>
        fGET_PROP_VALUES =
            [this](SCommandAttachmentImpl<cmdGET_PROP_VALUES>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdGET_PROP_VALUES(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_PROP_VALUES>(fGET_PROP_VALUES);

    function<bool(SCommandAttachmentImpl<cmdSET_TOPOLOGY>::ptr_t _attachment, CAgentChannel * _channel)> fSET_TOPOLOGY =
        [this](SCommandAttachmentImpl<cmdSET_TOPOLOGY>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdSET_TOPOLOGY(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdSET_TOPOLOGY>(fSET_TOPOLOGY);

    function<bool(SCommandAttachmentImpl<cmdREPLY_ID>::ptr_t _attachment, CAgentChannel * _channel)> fREPLY_ID = [this](
        SCommandAttachmentImpl<cmdREPLY_ID>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdREPLY_ID(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdREPLY_ID>(fREPLY_ID);

    function<bool(SCommandAttachmentImpl<cmdENABLE_STAT>::ptr_t _attachment, CAgentChannel * _channel)> fENABLE_STAT =
        [this](SCommandAttachmentImpl<cmdENABLE_STAT>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdENABLE_STAT(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdENABLE_STAT>(fENABLE_STAT);

    function<bool(SCommandAttachmentImpl<cmdDISABLE_STAT>::ptr_t _attachment, CAgentChannel * _channel)> fDISABLE_STAT =
        [this](SCommandAttachmentImpl<cmdDISABLE_STAT>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdDISABLE_STAT(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdDISABLE_STAT>(fDISABLE_STAT);

    function<bool(SCommandAttachmentImpl<cmdGET_STAT>::ptr_t _attachment, CAgentChannel * _channel)> fGET_STAT = [this](
        SCommandAttachmentImpl<cmdGET_STAT>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdGET_STAT(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdGET_STAT>(fGET_STAT);

    function<bool(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment, CAgentChannel * _channel)> fCUSTOM_CMD =
        [this](SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment, CAgentChannel* _channel) -> bool {
        return this->on_cmdCUSTOM_CMD(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdCUSTOM_CMD>(fCUSTOM_CMD);
}

//=============================================================================
void CConnectionManager::_createWnPkg(bool _needInlineBashScript) const
{
    // re-create the worker package if needed
    string out;
    string err;
    try
    {
        // set submit time
        chrono::system_clock::time_point now = chrono::system_clock::now();
        stringstream ssSubmitTime;
        ssSubmitTime << " -s " << chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        // invoking a new bash process can in some case overwrite env. vars
        // To be sure that our env is there, we call DDS_env.sh
        string cmd_env("$DDS_LOCATION/DDS_env.sh");
        smart_path(&cmd_env);

        string cmd("$DDS_LOCATION/bin/dds-prep-worker");
        smart_path(&cmd);
        cmd += ssSubmitTime.str();
        if (_needInlineBashScript)
            cmd += " -i";
        string arg("source ");
        arg += cmd_env;
        arg += " ; ";
        arg += cmd;

        stringstream ssCmd;
        ssCmd << "/bin/bash -c \"" << arg << "\"";

        LOG(debug) << "Preparing WN package: " << ssCmd.str();

        // 10 sec time-out for this command
        do_execv(ssCmd.str(), 10, &out, &err);
    }
    catch (exception& e)
    {
        stringstream ssErr;
        ssErr << "WN Package Tool: " << e.what() << "; STDOUT:" << out << "; STDERR: " << err;
        throw runtime_error(ssErr.str());
    }
    LOG(info) << "WN Package Tool: STDOUT:" << out << "; STDERR: " << err;
}

void CConnectionManager::_createInfoFile(const vector<size_t>& _ports) const
{
    const string sSrvCfg(CUserDefaults::instance().getServerInfoFileLocationSrv());
    LOG(MiscCommon::info) << "Creating the server info file: " << sSrvCfg;
    ofstream f(sSrvCfg.c_str());
    if (!f.is_open() || !f.good())
    {
        string msg("Could not open the server info configuration file: ");
        msg += sSrvCfg;
        throw runtime_error(msg);
    }

    string srvHost;
    MiscCommon::get_hostname(&srvHost);
    string srvUser;
    MiscCommon::get_cuser_name(&srvUser);

    if (_ports.size() > 0)
    {
        f << "[server]\n"
          << "host=" << srvHost << "\n"
          << "user=" << srvUser << "\n"
          << "port=" << _ports[0] << "\n"
          << endl;
    }

    if (_ports.size() > 1)
    {
        f << "[ui]\n"
          << "host=" << srvHost << "\n"
          << "user=" << srvUser << "\n"
          << "port=" << _ports[1] << "\n"
          << endl;
    }
}

void CConnectionManager::_deleteInfoFile() const
{
    const string sSrvCfg(CUserDefaults::instance().getServerInfoFileLocationSrv());
    if (sSrvCfg.empty())
        return;

    // TODO: check error code
    unlink(sSrvCfg.c_str());
}

bool CConnectionManager::on_cmdGET_LOG(SCommandAttachmentImpl<cmdGET_LOG>::ptr_t _attachment,
                                       CAgentChannel::weakConnectionPtr_t _channel)
{
    lock_guard<mutex> lock(m_getLog.m_mutexStart);

    if (!m_getLog.m_channel.expired())
    {
        auto p = _channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(
            SSimpleMsgCmd("Can not process the request. The getlog command is already in progress."));
        return true;
    }
    m_getLog.m_channel = _channel;
    m_getLog.zeroCounters();

    auto p = m_getLog.m_channel.lock();
    // Create directory to store logs
    const string sLogStorageDir(CUserDefaults::instance().getAgentLogStorageDir());
    fs::path dir(sLogStorageDir);
    if (!fs::exists(dir) && !fs::create_directory(dir))
    {
        stringstream ss;
        ss << "Could not create directory " << sLogStorageDir << " to save log files.";
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str()));

        m_getLog.m_channel.reset();
        return true;
    }

    auto condition = [](CAgentChannel::connectionPtr_t _v, bool& /*_stop*/) {
        return (_v->getChannelType() == EChannelType::AGENT && _v->started());
    };

    m_getLog.m_nofRequests = countNofChannels(condition);

    if (m_getLog.m_nofRequests == 0)
    {
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("There are no connected agents."));
        return true;
    }

    broadcastSimpleMsg<cmdGET_LOG>(condition);

    return true;
}

bool CConnectionManager::on_cmdBINARY_ATTACHMENT_RECEIVED(
    SCommandAttachmentImpl<cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
    CAgentChannel::weakConnectionPtr_t _channel)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdGET_LOG:
        {
            m_getLog.processMessage<SBinaryAttachmentReceivedCmd>(*_attachment, _channel);
            return true;
        }

        case cmdTRANSPORT_TEST:
        {
            m_transportTest.m_totalReceived += _attachment->m_receivedFileSize;
            m_transportTest.m_totalTime += _attachment->m_downloadTime;
            m_transportTest.processMessage<SBinaryAttachmentReceivedCmd>(*_attachment, _channel);

            return true;
        }
    }

    return true;
}

bool CConnectionManager::on_cmdSUBMIT(SCommandAttachmentImpl<cmdSUBMIT>::ptr_t _attachment,
                                      CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        auto p = _channel.lock();

        string pluginDir = CUserDefaults::instance().getPluginDir(_attachment->m_sPath, _attachment->m_sRMSType);
        stringstream ssPluginExe;
        ssPluginExe << pluginDir << "dds-submit-" << _attachment->m_sRMSType;
        if (!boost::filesystem::exists(ssPluginExe.str()))
        {
            stringstream ssErrMsg;
            ssErrMsg << "Unknown RMS plug-in requested \"" << _attachment->m_sRMSType << "\" (" << ssPluginExe.str()
                     << ")";
            p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ssErrMsg.str(), fatal, cmdSUBMIT));
            p->pushMsg<cmdSHUTDOWN>();
            return true;
        }

        // Submitting the job
        string outPut;
        stringstream ssCmd;
        ssCmd << ssPluginExe.str();
        // TODO: Send ID to the plug-in
        ssCmd << " --id "
              << "FAKE_ID_FOR_TESTS"
              << " --path \"" << pluginDir << "\"";

        // Create a new submit communication info channel
        lock_guard<mutex> lock(m_SubmitAgents.m_mutexStart);
        if (!m_SubmitAgents.m_channel.expired())
            throw runtime_error("Can not process the request. Submit is already in progress.");

        // Create / re-pack WN package
        // Include inline script if present
        string inlineShellScripCmds;
        if (!_attachment->m_sCfgFile.empty())
        {
            ifstream f(_attachment->m_sCfgFile);
            if (!f.is_open())
            {
                string msg("can't open configuration file \"");
                msg += _attachment->m_sCfgFile;
                msg += "\"";
                throw runtime_error(msg);
            }
            CNcf config;
            config.readFrom(f, true); // Read only bash commands if any
            inlineShellScripCmds = config.getBashEnvCmds();
            LOG(info)
                << "Agent submitter config contains an inline shell script. It will be injected it into wrk. package";

            string scriptFileName(CUserDefaults::instance().getWrkPkgDir());
            scriptFileName += "user_worker_env.sh";
            smart_path(&scriptFileName);
            ofstream f_script(scriptFileName.c_str());
            if (!f_script.is_open())
                throw runtime_error("Can't open for writing: " + scriptFileName);

            f_script << inlineShellScripCmds;
            f_script.close();
        }
        // pack worker package
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("Creating new worker package...", info, cmdSUBMIT));
        _createWnPkg(!inlineShellScripCmds.empty());

        // remember the UI channel, which requested to submit the job
        m_SubmitAgents.m_channel = _channel;
        m_SubmitAgents.zeroCounters();

        SSubmit submitRequest;
        submitRequest.m_cfgFilePath = _attachment->m_sCfgFile;
        submitRequest.m_nInstances = _attachment->m_nNumberOfAgents;
        submitRequest.m_wrkPackagePath = CUserDefaults::instance().getWrkScriptPath();
        m_SubmitAgents.m_strInitialSubmitRequest = submitRequest.toJSON();

        string sPluginInfoMsg("RMS plug-in: ");
        sPluginInfoMsg += ssPluginExe.str();
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(sPluginInfoMsg, info, cmdSUBMIT));

        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("Initializing RMS plug-in...", info, cmdSUBMIT));
        LOG(info) << "Calling RMS plug-in: " << ssCmd.str();

        // Let the submit info channel now, that plug-in is about to start
        m_SubmitAgents.initPlugin();

        try
        {
            do_execv(ssCmd.str()); //, 0 /*don't wait for plug-in*/, &outPut, nullptr, &nPluginExitCode);
        }
        catch (exception& e)
        {
            if (!outPut.empty())
            {
                ostringstream ss;
                ss << outPut;
                LOG(info) << ss.str();
                p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str()));
            }

            stringstream ssMsg;
            ssMsg << "Failed to deploy agents: " << e.what();
            throw runtime_error(ssMsg.str());
        }
        if (!outPut.empty())
        {
            ostringstream ss;
            ss << outPut;
            LOG(info) << ss.str();
            p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str()));
        }
    }
    catch (bad_weak_ptr& e)
    {
        // TODO: Do we need to log something here?

        // In case of any plugin initialization error, reset the info channel
        m_SubmitAgents.m_channel.reset();
    }
    catch (exception& e)
    {
        // In case of any plugin initialization error, reset the info channel
        m_SubmitAgents.m_channel.reset();

        if (!_channel.expired())
        {
            auto p = _channel.lock();
            p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(e.what(), fatal, cmdSUBMIT));
        }
    }

    return true;
}

bool CConnectionManager::on_cmdACTIVATE_AGENT(SCommandAttachmentImpl<cmdACTIVATE_AGENT>::ptr_t _attachment,
                                              CAgentChannel::weakConnectionPtr_t _channel)
{
    LOG(info) << "UI channel requested a topology activation...";
    lock_guard<mutex> lock(m_ActivateAgents.m_mutexStart);
    try
    {
        if (!m_ActivateAgents.m_channel.expired())
            throw runtime_error("Can not process the request. Activation of the agents is already in progress.");

        // remember the UI channel, which requested to submit the job
        m_ActivateAgents.m_channel = _channel;
        m_ActivateAgents.zeroCounters();

        auto p = m_ActivateAgents.m_channel.lock();
        // Start distributing user tasks between agents
        // TODO: We might need to create a thread here to avoid blocking a thread of the transport

        // Send binaries of user jobs to all active agents.
        // Send activate signal to all agents. This will trigger start of user jobs on the agents.
        auto condition = [](CAgentChannel::connectionPtr_t _v, bool& /*_stop*/) {
            return (_v->getChannelType() == EChannelType::AGENT && _v->started());
        };

        m_ActivateAgents.m_nofRequests = m_topo.getMainGroup()->getTotalNofTasks();
        size_t nofAgents = countNofChannels(condition);

        LOG(info) << "Available agents = " << nofAgents;

        if (nofAgents == 0)
            throw runtime_error("There are no connected agents.");
        if (nofAgents < m_ActivateAgents.m_nofRequests)
            throw runtime_error("The number of agents is not sufficient for this topology.");

        // initiate UI progress
        p->pushMsg<cmdPROGRESS>(SProgressCmd(0, m_ActivateAgents.m_nofRequests, 0));

        CAgentChannel::weakConnectionPtrVector_t channels(getChannels(condition));

        m_scheduler.makeSchedule(m_topo, channels);
        const CSSHScheduler::ScheduleVector_t& schedule = m_scheduler.getSchedule();

        // Clear the map on each activation
        m_taskIDToAgentChannelMap.clear();
        // Clear property tree
        // m_propertyPT.clear();

        for (const auto& sch : schedule)
        {
            SAssignUserTaskCmd msg_cmd;
            msg_cmd.m_taskID = sch.m_taskID;
            msg_cmd.m_taskIndex = sch.m_taskInfo.m_taskIndex;
            msg_cmd.m_collectionIndex = sch.m_taskInfo.m_collectionIndex;
            msg_cmd.m_taskPath = sch.m_taskInfo.m_taskPath;
            msg_cmd.m_groupName = sch.m_taskInfo.m_task->getParentGroupId();
            msg_cmd.m_collectionName = sch.m_taskInfo.m_task->getParentCollectionId();
            msg_cmd.m_taskName = sch.m_taskInfo.m_task->getId();

            if (sch.m_channel.expired())
                continue;
            auto ptr = sch.m_channel.lock();

            // Set task ID for agent and add it to map
            // TODO: Do we have to assign taskID here?
            // TODO: Probably it has to be assigned when the task is successfully activated.
            ptr->setTaskID(sch.m_taskID);
            m_taskIDToAgentChannelMap[sch.m_taskID] = sch.m_channel;

            if (sch.m_taskInfo.m_task->isExeReachable())
            {
                msg_cmd.m_sExeFile = sch.m_taskInfo.m_task->getExe();
            }
            else
            {
                // Executable is not reachable by the agent.
                // Upload it and change its path to $DDS_LOCATION on the WN

                // Expand the string for the program to extract exe name and command line arguments
                wordexp_t result;
                switch (wordexp(sch.m_taskInfo.m_task->getExe().c_str(), &result, 0))
                {
                    case 0:
                    {
                        string sExeFilePath = result.we_wordv[0];

                        boost::filesystem::path exeFilePath(sExeFilePath);
                        string sExeFileName = exeFilePath.filename().generic_string();

                        string sExeFileNameWithArgs = sExeFileName;
                        for (size_t i = 1; i < result.we_wordc; ++i)
                        {
                            sExeFileNameWithArgs += " ";
                            sExeFileNameWithArgs += result.we_wordv[i];
                        }

                        msg_cmd.m_sExeFile = "$DDS_LOCATION/";
                        msg_cmd.m_sExeFile += sExeFileNameWithArgs;

                        wordfree(&result);

                        //
                        ptr->pushBinaryAttachmentCmd(sExeFilePath, sExeFileName, cmdASSIGN_USER_TASK);
                    }
                    break;
                    case WRDE_NOSPACE:
                        // If the error was WRDE_NOSPACE,
                        // then perhaps part of the result was allocated.
                        throw runtime_error("memory error occurred while processing the user's executable path: " +
                                            sch.m_taskInfo.m_task->getExe());

                    case WRDE_BADCHAR:
                        throw runtime_error("Illegal occurrence of newline or one of |, &, ;, <, >, (, ), {, } in " +
                                            sch.m_taskInfo.m_task->getExe());
                        break;

                    case WRDE_BADVAL:
                        throw runtime_error("An undefined shell variable was referenced, and the WRDE_UNDEF flag "
                                            "told us to consider this an error in " +
                                            sch.m_taskInfo.m_task->getExe());
                        break;

                    case WRDE_CMDSUB:
                        throw runtime_error("Command substitution occurred, and the WRDE_NOCMD flag told us to "
                                            "consider this an error in " +
                                            sch.m_taskInfo.m_task->getExe());
                        break;
                    case WRDE_SYNTAX:
                        throw runtime_error(
                            "Shell syntax error, such as unbalanced parentheses or unmatched quotes in " +
                            sch.m_taskInfo.m_task->getExe());
                        break;

                    default: // Some other error.
                        throw runtime_error("failed to process the user's executable path: " +
                                            sch.m_taskInfo.m_task->getExe());
                }
            }
            ptr->pushMsg<cmdASSIGN_USER_TASK>(msg_cmd);
            ptr->setState(EAgentState::executing);
        }

        // Active agents.
        broadcastSimpleMsg<cmdACTIVATE_AGENT>([](CAgentChannel::connectionPtr_t _v, bool& /*_stop*/) {
            return (_v->getChannelType() == EChannelType::AGENT && _v->started() && _v->getTaskID() != 0);
        });
    }
    catch (exception& _e)
    {
        auto p = _channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdACTIVATE_AGENT));

        m_ActivateAgents.m_channel.reset();
        return true;
    }
    return true;
}

bool CConnectionManager::on_cmdSTOP_USER_TASK(SCommandAttachmentImpl<cmdSTOP_USER_TASK>::ptr_t _attachment,
                                              CAgentChannel::weakConnectionPtr_t _channel)
{
    lock_guard<mutex> lock(m_StopUserTasks.m_mutexStart);
    try
    {
        if (!m_StopUserTasks.m_channel.expired())
            throw runtime_error("Can not process the request. Stopping of tasks is already in progress.");

        // remember the UI channel, which requested to submit the job
        m_StopUserTasks.m_channel = _channel;
        m_StopUserTasks.zeroCounters();

        auto p = m_StopUserTasks.m_channel.lock();
        auto condition = [](CAgentChannel::connectionPtr_t _v, bool& /*_stop*/) {
            return (_v->getChannelType() == EChannelType::AGENT && _v->started() && _v->getTaskID() != 0);
        };

        m_StopUserTasks.m_nofRequests = countNofChannels(condition);

        // Stop UI if no tasks are running
        if (m_StopUserTasks.m_nofRequests <= 0)
        {
            p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("No running tasks. Nothing to stop.", fatal, cmdSTOP_USER_TASK));
            m_StopUserTasks.m_channel.reset();
            return true;
        }

        // initiate the progress on the UI
        p->pushMsg<cmdPROGRESS>(SProgressCmd(0, m_StopUserTasks.m_nofRequests, 0));

        CAgentChannel::weakConnectionPtrVector_t channels(getChannels(condition));
        for (const auto& v : channels)
        {
            if (v.expired())
                continue;
            auto ptr = v.lock();
            // remove task ID from the channel
            // ptr->setTaskID(0);
            // dequeue important (or expensive) messages
            ptr->dequeueMsg<cmdUPDATE_KEY>();
            ptr->dequeueMsg<cmdDELETE_KEY>();
            // send stop message
            ptr->template pushMsg<cmdSTOP_USER_TASK>();
            // ptr->setState(EAgentState::idle);
        }
    }
    catch (exception& _e)
    {
        auto p = _channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdSTOP_USER_TASK));

        m_StopUserTasks.m_channel.reset();
        return true;
    }
    return true;
}

bool CConnectionManager::on_cmdGET_AGENTS_INFO(SCommandAttachmentImpl<cmdGET_AGENTS_INFO>::ptr_t _attachment,
                                               CAgentChannel::weakConnectionPtr_t _channel)
{
    SAgentsInfoCmd cmd;
    stringstream ss;

    CAgentChannel::weakConnectionPtrVector_t channels(
        getChannels([](CAgentChannel::connectionPtr_t _v, bool& /*_stop*/) {
            return (_v->getChannelType() == EChannelType::AGENT && _v->started());
        }));

    for (const auto& v : channels)
    {
        if (v.expired())
            continue;
        auto ptr = v.lock();

        string sTaskName("no task is assigned");
        if (ptr->getTaskID() > 0)
        {
            TaskPtr_t task = m_topo.getTaskByHash(ptr->getTaskID());
            stringstream ssTaskString;
            ssTaskString << ptr->getTaskID() << " (" << task->getId() << ")";
            sTaskName = ssTaskString.str();
        }

        ++cmd.m_nActiveAgents;
        ss << " -------------->>> " << ptr->getId() << "\nHost Info: " << ptr->getRemoteHostInfo().m_username << "@"
           << ptr->getRemoteHostInfo().m_host << ":" << ptr->getRemoteHostInfo().m_DDSPath
           << "\nAgent pid: " << ptr->getRemoteHostInfo().m_agentPid
           << "\nAgent UI port: " << ptr->getRemoteHostInfo().m_agentPort
           << "\nAgent startup time: " << chrono::duration<double>(ptr->getStartupTime()).count() << " s"
           << "\nState: " << g_agentStates.at(ptr->getState()) << "\n"
           << "\nTask ID: " << sTaskName << "\n";
    }
    cmd.m_sListOfAgents = ss.str();

    if (!_channel.expired())
    {
        auto p = _channel.lock();
        p->pushMsg<cmdREPLY_AGENTS_INFO>(cmd);
    }

    return true;
}

bool CConnectionManager::on_cmdTRANSPORT_TEST(SCommandAttachmentImpl<cmdTRANSPORT_TEST>::ptr_t _attachment,
                                              CAgentChannel::weakConnectionPtr_t _channel)
{
    lock_guard<mutex> lock(m_transportTest.m_mutexStart);

    if (!m_transportTest.m_channel.expired())
    {
        auto p = _channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(
            "Can not process the request. The test command is already in progress.", fatal, cmdTRANSPORT_TEST));
        return true;
    }
    m_transportTest.m_channel = _channel;
    m_transportTest.zeroCounters();

    auto condition = [](CAgentChannel::connectionPtr_t _v, bool& /*_stop*/) {
        return (_v != nullptr && _v->getChannelType() == EChannelType::AGENT && _v->started());
    };

    vector<size_t> binarySizes{ 1000, 10000, 1000, 100000, 1000, 1000000, 1000, 10000000, 1000 };

    m_transportTest.m_nofRequests = binarySizes.size() * countNofChannels(condition);

    for (size_t size : binarySizes)
    {
        MiscCommon::BYTEVector_t data;
        for (size_t i = 0; i < size; ++i)
        {
            char c = rand() % 256;
            // char c = 'c';
            data.push_back(c);
        }

        string fileName = "test_data_" + to_string(size) + ".bin";
        broadcastBinaryAttachmentCmd(data, fileName, cmdTRANSPORT_TEST, condition);
    }

    if (m_transportTest.m_nofRequests == 0 && !m_transportTest.m_channel.expired())
    {
        auto p = m_transportTest.m_channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd("There are no active agents.", fatal, cmdTRANSPORT_TEST));
    }

    return true;
}

bool CConnectionManager::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                          CAgentChannel::weakConnectionPtr_t _channel)
{
    switch (_attachment->m_srcCommand)
    {
        case cmdACTIVATE_AGENT:
            switch (_attachment->m_msgSeverity)
            {
                case info:
                    m_ActivateAgents.processMessage<SSimpleMsgCmd>(*_attachment, _channel);
                    break;
                case error:
                case fatal:
                    m_ActivateAgents.processErrorMessage<SSimpleMsgCmd>(*_attachment, _channel);
                    break;
            }
            return true; // let others to process this message

        case cmdSTOP_USER_TASK:
            switch (_attachment->m_msgSeverity)
            {
                case info:
                    m_StopUserTasks.processMessage<SSimpleMsgCmd>(*_attachment, _channel);
                    break;
                case error:
                case fatal:
                    m_StopUserTasks.processErrorMessage<SSimpleMsgCmd>(*_attachment, _channel);
                    break;
            }
            return true; // let others to process this message

        case cmdGET_LOG:
            return m_getLog.processErrorMessage<SSimpleMsgCmd>(*_attachment, _channel);

        case cmdTRANSPORT_TEST:
            return m_transportTest.processErrorMessage<SSimpleMsgCmd>(*_attachment, _channel);
    }
    return false;
}

bool CConnectionManager::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                                          CAgentChannel::weakConnectionPtr_t _channel)
{
    // Update key-value from commander's key-value manager
    m_keyValueManager.updateKeyValue(*_attachment);

    // If UI channel sends a property update than property key does not contain a hash.
    // In this case each agent set the property key hash himself.
    auto channelPtr = _channel.lock();
    const bool sentFromUIChannel = (channelPtr->getChannelType() == EChannelType::UI);

    string propertyID(_attachment->m_sKey);
    if (!sentFromUIChannel)
    {
        // If we get property key from agent we have to parse it to get the property ID.
        // propertyID.17621121989812
        propertyID = _attachment->getPropertyID();
    }

    // Check if the property has a write access to property.
    // If not just send back an error.
    if (!sentFromUIChannel)
    {
        auto task = m_topo.getTaskByHash(channelPtr->getTaskID());
        auto property = task->getProperty(propertyID);
        if (property == nullptr || (property != nullptr && (property->getAccessType() == EPropertyAccessType::READ)))
        {
            stringstream ss;
            if (property == nullptr)
                ss << "Can't propagate property <" << propertyID << "> that doesn't exist for task " << task->getId();
            else
                ss << "Can't propagate property <" << property->getId() << "> which has a READ access type for task "
                   << task->getId();
            channelPtr->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, cmdUPDATE_KEY));
        }
    }

    CTopology::TaskInfoIteratorPair_t taskIt = m_topo.getTaskInfoIteratorForPropertyId(propertyID);

    for (auto it = taskIt.first; it != taskIt.second; ++it)
    {
        // We have to lock with m_mapMutex here. The map can be changed in on_cmdUSER_TASK_DONE.
        // TODO: For the moment lock is removed because it introduces the bottle neck.
        // TODO: In on_cmdUSER_TASK_DONE function we do not remove anything from m_taskIDToAgentChannelMap map.
        bool iterExists = false;
        CAgentChannel::weakConnectionPtr_t weakPtr;
        //{
        //    lock_guard<mutex> lock(m_mapMutex);
        auto iter = m_taskIDToAgentChannelMap.find(it->first);
        iterExists = (iter != m_taskIDToAgentChannelMap.end());
        if (iterExists)
            weakPtr = iter->second;
        //}

        if (!iterExists)
        {
            LOG(debug) << "on_cmdUPDATE_KEY task <" << it->first << "> not found in map. Property will not be updated.";
        }
        else
        {
            if (weakPtr.expired())
                continue;
            auto ptr = weakPtr.lock();

            if (ptr->getChannelType() == EChannelType::AGENT && ptr->started())
            {
                if (sentFromUIChannel)
                {
                    // If property changed from UI we have to change hash in the property key.
                    SUpdateKeyCmd attachment(*_attachment);
                    attachment.setKey(propertyID, ptr->getTaskID());
                    ptr->pushMsg<cmdUPDATE_KEY>(attachment);
                    LOG(info) << "Property update from UI channel: " << attachment.m_sKey;
                }
                else
                {
                    if (ptr->getTaskID() != channelPtr->getTaskID())
                    {
                        auto task = m_topo.getTaskByHash(ptr->getTaskID());
                        auto property = task->getProperty(propertyID);
                        if (property != nullptr && (property->getAccessType() == EPropertyAccessType::READ ||
                                                    property->getAccessType() == EPropertyAccessType::READWRITE))
                        {
                            // ptr->pushMsg<cmdUPDATE_KEY>(*_attachment);
                            ptr->accumulativePushMsg<cmdUPDATE_KEY>(*_attachment);
                            LOG(debug) << "Property update from agent channel: <" << *_attachment << ">";
                        }
                    }
                }
            }
        }
    }

    channelPtr->pushMsg<cmdSIMPLE_MSG>(
        SSimpleMsgCmd("All related agents have been advised about the key update.", MiscCommon::debug, cmdUPDATE_KEY));
    // Shutdown the initial channel if it's an UI one
    if (sentFromUIChannel)
    {
        LOG(debug) << "A key-value notification has been broadcasted:" << *_attachment
                   << " There are no more requests from the UI channel, therefore shutting it down.";
        channelPtr->pushMsg<cmdSHUTDOWN>();
    }

    return true;
}

bool CConnectionManager::on_cmdUSER_TASK_DONE(SCommandAttachmentImpl<cmdUSER_TASK_DONE>::ptr_t _attachment,
                                              CAgentChannel::weakConnectionPtr_t _channel)
{
    uint64_t taskID = _attachment->m_taskID;
    // Delete key-value from commander's key-value manager
    m_keyValueManager.deleteKeyValue(taskID);

    auto task = m_topo.getTaskByHash(taskID);

    const TopoPropertyPtrVector_t& properties = task->getProperties();
    for (const auto& property : properties)
    {
        CTopology::TaskInfoIteratorPair_t taskIt = m_topo.getTaskInfoIteratorForPropertyId(property->getId());

        for (auto it = taskIt.first; it != taskIt.second; ++it)
        {
            auto iter = m_taskIDToAgentChannelMap.find(it->first);
            if (iter == m_taskIDToAgentChannelMap.end())
            {
                LOG(debug) << "on_cmdDELETE_KEY task <" << it->first
                           << "> not found in map. Property will not be deleted.";
            }
            else
            {
                if (iter->second.expired())
                    continue;
                auto ptr = iter->second.lock();

                SDeleteKeyCmd cmd;
                cmd.setKey(property->getId(), taskID);
                if (ptr->getTaskID() != 0 && ptr->getTaskID() != taskID)
                {
                    ptr->pushMsg<cmdDELETE_KEY>(cmd);
                    LOG(debug) << "Property deleted from agent channel: <" << *_attachment << ">";
                }
            }
        }
    }

    if (!_channel.expired())
    {
        auto channelPtr = _channel.lock();
        // remove task ID from the channel
        channelPtr->setTaskID(0);
        channelPtr->setState(EAgentState::idle);
    }

    // remove task ID from the map
    // TODO: Temporary solution. We do not remove tasks from the map to avoid synchronization bottlenecks in
    // on_cmdUPDATE_KEY.
    //{
    //    lock_guard<mutex> lock(m_mapMutex);
    //    auto it = m_taskIDToAgentChannelMap.find(taskID);
    //    if (it != m_taskIDToAgentChannelMap.end())
    //        m_taskIDToAgentChannelMap.erase(it);
    //}

    LOG(info) << "User task <" << taskID << "> with path " << task->getPath() << " done";

    return true;
}

bool CConnectionManager::on_cmdGET_PROP_LIST(SCommandAttachmentImpl<cmdGET_PROP_LIST>::ptr_t _attachment,
                                             CAgentChannel::weakConnectionPtr_t _channel)
{
    if (!_channel.expired())
    {
        auto ptr = _channel.lock();
        ptr->pushMsg<cmdSIMPLE_MSG>(
            SSimpleMsgCmd(m_keyValueManager.getPropertyString(), MiscCommon::info, cmdGET_PROP_LIST));
    }

    return true;
}

bool CConnectionManager::on_cmdGET_PROP_VALUES(SCommandAttachmentImpl<cmdGET_PROP_VALUES>::ptr_t _attachment,
                                               CAgentChannel::weakConnectionPtr_t _channel)
{
    try
    {
        if (!_channel.expired())
        {
            auto ptr = _channel.lock();
            ptr->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(
                m_keyValueManager.getKeyValueString(_attachment->m_sPropertyID), MiscCommon::info, cmdGET_PROP_VALUES));
        }
    }
    catch (exception& e)
    {
        if (!_channel.expired())
        {
            stringstream ss;
            ss << "Error getting values for property " << _attachment->m_sPropertyID << ": " << e.what();
            auto ptr = _channel.lock();
            ptr->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::error, cmdGET_PROP_VALUES));
        }
    }

    return true;
}

bool CConnectionManager::on_cmdSET_TOPOLOGY(SCommandAttachmentImpl<cmdSET_TOPOLOGY>::ptr_t _attachment,
                                            CAgentChannel::weakConnectionPtr_t _channel)
{
    LOG(info) << "UI channel requested to set up a new ropology. " << *_attachment;
    auto p = _channel.lock();
    try
    {
        // Resolve topology
        m_topo.setXMLValidationDisabled(_attachment->m_nDisiableValidation);
        m_topo.init(_attachment->m_sTopologyFile);
        m_keyValueManager.initWithTopology(m_topo);
        auto p = _channel.lock();
        p->pushMsg<cmdSIMPLE_MSG>(
            SSimpleMsgCmd("new Topology is set to: " + _attachment->m_sTopologyFile, info, cmdSET_TOPOLOGY));
    }
    catch (exception& _e)
    {
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdSET_TOPOLOGY));
    }
    p->pushMsg<cmdSHUTDOWN>();
    return true;
}

bool CConnectionManager::on_cmdREPLY_ID(SCommandAttachmentImpl<cmdREPLY_ID>::ptr_t _attachment,
                                        CAgentChannel::weakConnectionPtr_t _channel)
{
    auto p = _channel.lock();
    try
    {
        LOG(debug) << "cmdREPLY_ID attachment [" << *_attachment << "] received from: " << p->remoteEndIDString();

        if (_attachment->m_id == 0)
        {
            uint64_t agentId = DDSChannelId::getChannelId();
            p->setId(agentId);
            SIDCmd msg_cmd;
            msg_cmd.m_id = p->getId();
            p->pushMsg<cmdSET_ID>(msg_cmd);
        }
        else
        {
            p->setId(_attachment->m_id);
        }
    }
    catch (exception& _e)
    {
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdREPLY_ID));
    }

    return true;
}

void CConnectionManager::enableDisableStatForChannels(bool _enable)
{
    CAgentChannel::weakConnectionPtrVector_t channels(getChannels());
    //                                                      [](CAgentChannel::connectionPtr_t _v)
    //                    {
    //                        return (_v->getChannelType() == EChannelType::AGENT && _v->started());
    //                    }));

    for (const auto& v : channels)
    {
        if (v.expired())
            continue;
        auto ptr = v.lock();

        ptr->setStatEnabled(_enable);
    }

    m_statEnabled = _enable;
}

bool CConnectionManager::on_cmdENABLE_STAT(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdENABLE_STAT>::ptr_t _attachment,
    CAgentChannel::weakConnectionPtr_t _channel)
{
    auto p = _channel.lock();
    try
    {
        enableDisableStatForChannels(true);

        p->pushMsg<cmdSIMPLE_MSG>(
            SSimpleMsgCmd("Statistics is enabled on DDS commander server", MiscCommon::info, cmdENABLE_STAT));
    }
    catch (exception& _e)
    {
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdENABLE_STAT));
    }

    return true;
}

bool CConnectionManager::on_cmdDISABLE_STAT(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdDISABLE_STAT>::ptr_t _attachment,
    CAgentChannel::weakConnectionPtr_t _channel)
{
    auto p = _channel.lock();
    try
    {
        enableDisableStatForChannels(false);

        p->pushMsg<cmdSIMPLE_MSG>(
            SSimpleMsgCmd("Statistics is disabled on DDS commander server", MiscCommon::info, cmdDISABLE_STAT));
    }
    catch (exception& _e)
    {
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdDISABLE_STAT));
    }

    return true;
}

bool CConnectionManager::on_cmdGET_STAT(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_STAT>::ptr_t _attachment,
    CAgentChannel::weakConnectionPtr_t _channel)
{
    auto p = _channel.lock();
    try
    {
        CAgentChannel::weakConnectionPtrVector_t channels(getChannels());
        //        [](CAgentChannel::connectionPtr_t _v)
        //                        {
        //                            return (_v->getChannelType() == EChannelType::AGENT && _v->started());
        //                        }));

        SReadStat readStat;
        SWriteStat writeStat;

        for (const auto& v : channels)
        {
            if (v.expired())
                continue;
            auto ptr = v.lock();

            readStat.addFromStat(ptr->getReadStat());
            writeStat.addFromStat(ptr->getWriteStat());
        }

        addDisconnectedChannelsStatToStat(readStat, writeStat);

        stringstream ss;
        ss << "Number of active channels: " << channels.size() << endl << readStat.toString() << writeStat.toString();

        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::info, cmdGET_STAT));
    }
    catch (exception& _e)
    {
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdGET_STAT));
    }

    return true;
}

bool CConnectionManager::on_cmdCUSTOM_CMD(
    protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment,
    CAgentChannel::weakConnectionPtr_t _channel)
{
    auto p = _channel.lock();
    try
    {
        // Assign sender ID of this custom command
        _attachment->m_senderId = p->getId();

        CAgentChannel::weakConnectionPtrVector_t channels;

        // First check id condition is a positive integer - channel ID - which means that custom command has to be sent
        // to a particular channel.
        try
        {
            // If condition in the attachment is not of type uint64_t function throws an exception.
            uint64_t channelId = boost::lexical_cast<uint64_t>(_attachment->m_sCondition);
            channels = getChannels([channelId](CAgentChannel::connectionPtr_t _v, bool& _stop) {
                _stop = (_v->getId() == channelId);
                return _stop;
            });
        }
        catch (boost::bad_lexical_cast&)
        {
            // Condition is not a positiove integer.

            // check if this is an agent submit request
            if (_attachment->m_sCondition == g_sRmsAgentSign)
            {
                LOG(info) << "Received a message from RMS plug-in.";
                lock_guard<mutex> lock(m_SubmitAgents.m_mutexStart);
                if (m_SubmitAgents.m_channel.expired())
                    throw runtime_error("Internal error. Submit info channel is not initialized.");

                // Remember the submit plug-in channel, which is responsible for job submissions
                // Send initial request
                if (m_SubmitAgents.m_channelSubmitPlugin.expired())
                    m_SubmitAgents.m_channelSubmitPlugin = _channel;

                // Process messages from the plug-in
                m_SubmitAgents.processCustomCommandMessage(*_attachment, _channel);
                return true;
            }

            // Check if we can find task for it's full hash path.
            bool taskFound = true;
            try
            {
                TaskPtr_t task = m_topo.getTaskByHashPath(_attachment->m_sCondition);
            }
            catch (runtime_error& _e)
            {
                taskFound = false;
            }

            channels = getChannels([this, taskFound, &_attachment, p](CAgentChannel::connectionPtr_t _v, bool& _stop) {
                // Only for Agents which are started already and executing task
                if (_v->getChannelType() != EChannelType::AGENT || !_v->started() ||
                    _v->getState() != EAgentState::executing)
                    return false;

                // Do not send command to self
                if (_v->getTaskID() == p->getTaskID())
                    return false;

                // If condition is empty we broadcast command to all agents
                if (_attachment->m_sCondition.empty())
                    return true;

                const STaskInfo& taskInfo = m_topo.getTaskInfoByHash(_v->getTaskID());
                bool result = (taskFound) ? taskInfo.m_taskPath == _attachment->m_sCondition
                                          : taskInfo.m_task->getPath() == _attachment->m_sCondition;

                _stop = (taskFound && result);

                return result;
            });
        }

        for (const auto& v : channels)
        {
            if (v.expired())
                continue;
            auto ptr = v.lock();

            ptr->pushMsg<cmdCUSTOM_CMD>(*_attachment);
        }

        stringstream ss;
        ss << "Send custom command to " << channels.size() << " channels." << endl;

        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(ss.str(), MiscCommon::info, cmdCUSTOM_CMD));
    }
    catch (exception& _e)
    {
        LOG(error) << "on_cmdCUSTOM_CMD: " << _e.what();
        p->pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(_e.what(), fatal, cmdCUSTOM_CMD));
    }

    return true;
}
