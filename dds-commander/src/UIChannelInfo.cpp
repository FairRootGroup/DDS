// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "UIChannelInfo.h"

using namespace std;
using namespace dds;
using namespace dds::commander_cmd;

CSubmitAgentsChannelInfo::CSubmitAgentsChannelInfo()
    : CUIChannelInfo<CSubmitAgentsChannelInfo>()
    , m_PluginStartTime(chrono::system_clock::duration::zero())
    , m_bInit(false)
{
    m_srcCommand = protocol_api::cmdSUBMIT;
}

string CSubmitAgentsChannelInfo::getMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                            CAgentChannel::weakConnectionPtr_t _channel) const
{
    stringstream ss;
    auto p = _channel.lock();
    ss << _cmd.m_sMsg;
    return ss.str();
}

string CSubmitAgentsChannelInfo::getErrorMessage(const protocol_api::SSimpleMsgCmd& _cmd,
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

bool CSubmitAgentsChannelInfo::processCustomCommandMessage(const protocol_api::SCustomCmdCmd& _cmd,
                                                           CAgentChannel::weakConnectionPtr_t _channel)
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
                pPlugin->subscribeOnEvent(protocol_api::EChannelEvents::OnRemoteEndDissconnected,
                                          [this](CAgentChannel* _channel) {
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
                pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(
                    protocol_api::SSimpleMsgCmd(ssMsg.str(), MiscCommon::info));

                intercom_api::SInit init;
                init.fromPT(pt);
                // Sending Submit request to the plug-in
                protocol_api::SCustomCmdCmd cmd;
                cmd.m_sCmd = m_strInitialSubmitRequest;
                cmd.m_sCondition = "";
                pPlugin->template pushMsg<protocol_api::cmdCUSTOM_CMD>(cmd);
            }
            else if (tag == "message")
            {
                intercom_api::SMessage message;
                message.fromPT(pt);
                stringstream ss;
                ss << "Plug-in: " << message.m_msg;
                pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(protocol_api::SSimpleMsgCmd(
                    ss.str(),
                    (message.m_msgSeverity == intercom_api::EMsgSeverity::info ? MiscCommon::info
                                                                               : MiscCommon::error)));
                // Drop the connection if received an error message from plug-in
                if (message.m_msgSeverity == intercom_api::EMsgSeverity::error)
                {
                    pUI->template pushMsg<protocol_api::cmdSHUTDOWN>();
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

        auto pUI = m_channel.lock();
        if (pUI)
            pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(protocol_api::SSimpleMsgCmd(msg, MiscCommon::error));

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

    // Give plug-in 2 minute to start
    auto now = chrono::system_clock::now().time_since_epoch();
    auto diff = now - m_PluginStartTime;
    if (diff >= chrono::minutes(2))
    {
        if (!m_channel.expired())
        {
            auto pUI = m_channel.lock();
            if (pUI)
                pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(
                    protocol_api::SSimpleMsgCmd("Plug-in failed to start.", MiscCommon::error));
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
            auto pUI = m_channel.lock();
            if (pUI)
                pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(
                    protocol_api::SSimpleMsgCmd(ss.str(), MiscCommon::info));
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
            auto pUI = m_channel.lock();
            if (pUI)
                pUI->template pushMsg<protocol_api::cmdSHUTDOWN>();
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
                pPlugin->template pushMsg<protocol_api::cmdSHUTDOWN>();
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
