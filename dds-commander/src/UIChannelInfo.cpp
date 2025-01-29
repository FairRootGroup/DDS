// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "UIChannelInfo.h"
#include "Intercom.h"
#include "UserDefaults.h"
#include "submit_info.pb.h"
#include "submit_info_slurm.pb.h"
#include <boost/filesystem.hpp>

using namespace std;
using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::protocol;
using namespace dds::protocol_api;
using namespace dds::misc;

const LPCSTR g_submitInfoFile = "submit.inf";

CSubmitAgentsChannelInfo::CSubmitAgentsChannelInfo()
    : CUIChannelInfo<CSubmitAgentsChannelInfo>()
    , m_PluginStartTime(chrono::system_clock::duration::zero())
    , m_bInit(false)
{
    m_srcCommand = cmdSUBMIT;
}

string CSubmitAgentsChannelInfo::getMessage(const SSimpleMsgCmd& _cmd,
                                            CAgentChannel::weakConnectionPtr_t _channel) const
{
    stringstream ss;
    auto p = _channel.lock();
    ss << _cmd.m_sMsg;
    return ss.str();
}

string CSubmitAgentsChannelInfo::getErrorMessage(const SSimpleMsgCmd& _cmd,
                                                 CAgentChannel::weakConnectionPtr_t _channel) const
{
    stringstream ss;
    auto p = _channel.lock();
    ss << _cmd.m_sMsg;
    return ss.str();
}

string CSubmitAgentsChannelInfo::getAllReceivedMessage() const
{
    stringstream ss;
    // ss << "total: " << m_nofRequests << ", activations: " << nofReceived()
    // << ", errors: " << m_nofReceivedErrors << "\n";

    return ss.str();
}

bool CSubmitAgentsChannelInfo::processCustomCommandMessage(const SCustomCmdCmd& _cmd,
                                                           CAgentChannel::weakConnectionPtr_t /*_channel*/)
{
    LOG(info) << "Processing RMS plug-in message: " << _cmd.m_sCmd;
    boost::property_tree::ptree pt;

    try
    {
        lock_guard<mutex> lock(m_mutexReceive);

        // Early exit checks
        if (m_channel.expired())
        {
            LOG(fatal) << "Can't process RMS plug-in messages. UI channel expired";
            return true;
        }
        if (m_channelSubmitPlugin.expired())
        {
            LOG(fatal) << "Can't process RMS plug-in messages. Plug-in channel expired";
            return true;
        }

        // lock channels
        auto pUI = m_channel.lock();
        auto pPlugin = m_channelSubmitPlugin.lock();
        if (!pUI || !pPlugin)
        {
            LOG(fatal) << "Can't process RMS plug-in messages. Channel locks failed";
            return true;
        }

        istringstream ss(_cmd.m_sCmd);
        boost::property_tree::read_json(ss, pt);

        const boost::property_tree::ptree& childPT = pt.get_child("dds.plug-in");

        for (const auto& child : childPT)
        {
            const string& tag = child.first;
            if (tag == "init")
            {
                // Subscribe on plug-in disconnect
                pPlugin->registerHandler<EChannelEvents::OnRemoteEndDissconnected>(
                    [this](const SSenderInfo& /*_sender*/)
                    {
                        LOG(info) << "Plug-in disconnect subscription called";
                        m_channelSubmitPlugin.reset();
                        shutdown();
                    });

                auto now = chrono::system_clock::now().time_since_epoch();
                auto diff = now - m_PluginStartTime;
                stringstream ssMsg;
                ssMsg << "RMS plug-in is online. Startup time: "
                      << chrono::duration_cast<chrono::milliseconds>(diff).count() << "ms.";
                LOG(info) << ssMsg.str();
                sendUIMessage(ssMsg.str());

                intercom_api::SInit init;
                init.fromPT(pt);

                // Store submission ID from the init message
                m_sSubmissionID = init.m_id;

                // Sending Submit request to the plug-in
                SCustomCmdCmd cmd;
                cmd.m_sCmd = m_strInitialSubmitRequest;
                cmd.m_sCondition = "";
                pPlugin->template pushMsg<cmdCUSTOM_CMD>(cmd);
            }
            else if (tag == "message")
            {
                intercom_api::SMessage message;
                message.fromPT(pt);
                stringstream ss;
                ss << "Plug-in: " << message.m_msg;
                sendUIMessage(ss.str(),
                              (message.m_msgSeverity == dds::intercom_api::EMsgSeverity::info
                                   ? dds::intercom_api::EMsgSeverity::info
                                   : dds::intercom_api::EMsgSeverity::error));

                // FIXME: This is a temporary solution.
                // When we receive "DDS agents have been submitted" message, we parse submit.inf file
                if (message.m_msg == "DDS agents have been submitted")
                {
                    fs::path pathWorkDirLocalFiles(
                        smart_path(user_defaults_api::CUserDefaults::instance().getValueForKey("server.work_dir")));
                    pathWorkDirLocalFiles /= m_sSubmissionID; // Use submission ID instead of channel ID
                    fs::path pathSubmitInfo(pathWorkDirLocalFiles);
                    pathSubmitInfo /= g_submitInfoFile;

                    if (fs::exists(pathSubmitInfo))
                    {
                        fstream input(pathSubmitInfo.native(), ios::in | ios::binary);
                        dds::protocol::SubmitInfo protoSubmitInfo;
                        if (!protoSubmitInfo.ParseFromIstream(&input))
                        {
                            throw runtime_error("Failed to parse submission info");
                        }

                        // Read RMS specific data
                        dds::protocol::SlurmSubmitInfo protoSlurmSubmitInfo;
                        if (protoSubmitInfo.rms_plugin_data().UnpackTo(&protoSlurmSubmitInfo))
                        {
                            tools_api::SSubmitResponseData submitResponse;
                            submitResponse.m_requestID = m_requestID;

                            // Copy all job IDs from the protobuf repeated field
                            for (int i = 0; i < protoSlurmSubmitInfo.slurm_job_id_size(); ++i)
                            {
                                submitResponse.m_jobIDs.push_back(protoSlurmSubmitInfo.slurm_job_id(i));
                            }

                            submitResponse.m_allocNodes = protoSlurmSubmitInfo.alloc_nodes();
                            submitResponse.m_state = protoSlurmSubmitInfo.state();
                            submitResponse.m_jobInfoAvailable = protoSlurmSubmitInfo.job_info_available();

                            // Add logging to ensure the response is being sent
                            LOG(info) << "Sending submit response to UI...";
                            // Send response via custom command
                            SCustomCmdCmd cmd;
                            cmd.m_sCmd = submitResponse.toJSON();
                            cmd.m_sCondition = "";
                            pUI->pushMsg<cmdCUSTOM_CMD>(cmd);
                        }
                    }
                }

                if (message.m_msgSeverity == dds::intercom_api::EMsgSeverity::error)
                {
                    doneWithUI();
                    m_channel.reset();
                }
            }
            else if (tag == "submit")
            {
                doneWithUI();
            }
        }
    }
    catch (const boost::property_tree::ptree_error& _e)
    {
        string msg("Failed to process RMS plug-in message: ");
        msg += _e.what();
        LOG(error) << msg;
        sendUIMessage(msg, dds::intercom_api::EMsgSeverity::error);
        shutdown();
    }
    catch (const exception& _e)
    {
        LOG(error) << "CSubmitAgentsChannelInfo::processCustomCommandMessage: " << _e.what();
        shutdown();
    }
    return true;
}

bool CSubmitAgentsChannelInfo::isPluginOnline()
{
    return (!m_channel.expired());
}

// This function can be called to check whether plug-in failed to start.
// If this is the case, the function will alert user and close connection to UI.
void CSubmitAgentsChannelInfo::checkPluginFailedToStart()
{
    lock_guard<mutex> lock(m_mutexReceive);
    // plug-in has not been yet started
    if (chrono::duration_cast<chrono::milliseconds>(m_PluginStartTime) == chrono::system_clock::duration::zero() &&
        !m_bInit)
    {
        return;
    }

    //                if (m_bInit && m_channelSubmitPlugin.expired())
    //                {
    //                    // the plug-in is done and went offline
    //                    shutdown();
    //                    return;
    //                }

    // plug-in is online
    if (!m_channelSubmitPlugin.expired())
    {
        // plug-in is online
        // reset the start timer
        m_PluginStartTime = chrono::system_clock::duration::zero();
        return;
    }

    // Give plug-in 1 minutes to start
    auto now = chrono::system_clock::now().time_since_epoch();
    auto diff = now - m_PluginStartTime;
    if (diff >= chrono::minutes(1))
    {
        if (!m_channel.expired())
        {
            sendUIMessage("Plug-in failed to start.", dds::intercom_api::EMsgSeverity::error);
            shutdown();
        }
    }
    else
    {
        if (!m_channel.expired())
        {
            stringstream ss;
            ss << "still waiting for plug-in (elapsed " << chrono::duration_cast<chrono::seconds>(diff).count()
               << "s)...";
            sendUIMessage(ss.str());
        }
    }
}

void CSubmitAgentsChannelInfo::shutdown()
{
    m_bInit = false;
    try
    {
        if (!m_channel.expired())
        {
            doneWithUI();
            m_channel.reset();
        }
    }
    catch (...)
    {
    }

    try
    {
        if (!m_channelSubmitPlugin.expired())
        {
            auto pPlugin = m_channel.lock();
            if (pPlugin)
                pPlugin->template pushMsg<cmdSHUTDOWN>();
            m_channelSubmitPlugin.reset();
        }
    }
    catch (...)
    {
    }
}

void CSubmitAgentsChannelInfo::initPlugin()
{
    m_bInit = true;
    m_PluginStartTime = chrono::system_clock::now().time_since_epoch();
}
