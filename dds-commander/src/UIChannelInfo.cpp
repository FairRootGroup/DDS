// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "UIChannelInfo.h"
#include "Intercom.h"

using namespace std;
using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::protocol_api;

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
    LOG(MiscCommon::info) << "Processing RMS plug-in message: " << _cmd.m_sCmd;
    boost::property_tree::ptree pt;

    try
    {
        lock_guard<mutex> lock(m_mutexReceive);

        if (m_channel.expired())
        {
            LOG(MiscCommon::fatal) << "Can't process RMS plug-in messages. UI channel expired";
            return true;
        }
        if (m_channelSubmitPlugin.expired())
        {
            LOG(MiscCommon::fatal) << "Can't process RMS plug-in messages. Plug-in channel expired";
            return true;
        }

        // lock UI channel
        auto pUI = m_channel.lock();
        if (!pUI)
        {
            LOG(MiscCommon::fatal) << "Can't process RMS plug-in messages. UI channel can't be locked";
            return true;
        }
        // lock Plug-in channel
        auto pPlugin = m_channelSubmitPlugin.lock();
        if (!pPlugin)
        {
            LOG(MiscCommon::fatal) << "Can't process RMS plug-in messages. Plug-in channel can't be locked";
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
                        LOG(MiscCommon::info) << "Plug-in disconnect subscription called";
                        // the plug-in is done and went offline, let's close UI
                        // connection as well.
                        m_channelSubmitPlugin.reset();
                        shutdown();
                    });

                auto now = chrono::system_clock::now().time_since_epoch();
                auto diff = now - m_PluginStartTime;
                stringstream ssMsg;
                ssMsg << "RMS plug-in is online. Startup time: "
                      << chrono::duration_cast<chrono::milliseconds>(diff).count() << "ms.";
                LOG(MiscCommon::info) << ssMsg.str();
                sendUIMessage(ssMsg.str());

                intercom_api::SInit init;
                init.fromPT(pt);
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

                // Drop the connection if received an error message from plug-in
                if (message.m_msgSeverity == dds::intercom_api::EMsgSeverity::error)
                {
                    doneWithUI();
                    m_channel.reset();
                }
            }
        }
    }
    // catch explicitly ptree_error. On some systems the exact exception message is not propagated to
    // exception parent and can be lost
    catch (const boost::property_tree::ptree_error& _e)
    {
        string msg("Failed to process RMS plug-in message: ");
        msg += _e.what();
        LOG(MiscCommon::error) << msg;

        sendUIMessage(msg, dds::intercom_api::EMsgSeverity::error);
        shutdown();
    }
    catch (const exception& _e)
    {
        LOG(MiscCommon::error) << "CSubmitAgentsChannelInfo::processCustomCommandMessage: ";
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
