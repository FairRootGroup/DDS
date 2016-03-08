// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__UIChannelInfo__
#define __DDS__UIChannelInfo__

// DDS
#include "AgentChannel.h"
#include "CustomCmdCmd.h"
#include "ProtocolCommands.h"
#include "dds_intercom.h"
// STD
#include <chrono>
#include <mutex>
#include <sstream>
#include <string>
// BOOST
#include <boost/property_tree/json_parser.hpp>

namespace dds
{
    namespace commander_cmd
    {
        template <class T>
        class CUIChannelInfo
        {

          public:
            CUIChannelInfo()
                : m_nofRequests(0)
                , m_nofReceived(0)
                , m_nofReceivedErrors(0)
                , m_mutexStart()
                , m_mutexReceive()
                , m_startTime(std::chrono::steady_clock::now())
            {
            }

          protected:
            size_t nofReceived() const
            {
                return (m_nofReceived + m_nofReceivedErrors);
            }

            bool allReceived() const
            {
                return (nofReceived() == m_nofRequests);
            }

          public:
            void zeroCounters()
            {
                m_nofRequests = 0;
                m_nofReceived = 0;
                m_nofReceivedErrors = 0;
                m_startTime = std::chrono::steady_clock::now();
            }

          public:
            template <class A>
            bool processMessage(const A& _cmd, CAgentChannel::weakConnectionPtr_t _channel)
            {
                try
                {
                    std::lock_guard<std::mutex> lock(m_mutexReceive);

                    ++m_nofReceived;

                    T* pThis = static_cast<T*>(this);
                    std::string userMessage = pThis->getMessage(_cmd, _channel);

                    if (!m_channel.expired())
                    {
                        auto pUI = m_channel.lock();
                        pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(
                            protocol_api::SSimpleMsgCmd(userMessage, MiscCommon::info));

                        // measure time to activate
                        std::chrono::steady_clock::time_point curTime = std::chrono::steady_clock::now();

                        pUI->template pushMsg<protocol_api::cmdPROGRESS>(protocol_api::SProgressCmd(
                            m_nofReceived,
                            m_nofRequests,
                            m_nofReceivedErrors,
                            std::chrono::duration_cast<std::chrono::milliseconds>(curTime - m_startTime).count()));
                    }

                    checkAllReceived();
                }
                catch (std::bad_weak_ptr& e)
                {
                    // TODO: Do we need to log something here?
                }

                return true;
            }

            template <class A>
            bool processErrorMessage(const A& _cmd, CAgentChannel::weakConnectionPtr_t _channel)
            {
                try
                {
                    std::lock_guard<std::mutex> lock(m_mutexReceive);

                    ++m_nofReceivedErrors;

                    T* pThis = static_cast<T*>(this);
                    std::string userMessage = pThis->getErrorMessage(_cmd, _channel);

                    if (!m_channel.expired())
                    {
                        auto pUI = m_channel.lock();
                        pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(
                            protocol_api::SSimpleMsgCmd(userMessage, MiscCommon::error));

                        // measure time to activate
                        std::chrono::steady_clock::time_point curTime = std::chrono::steady_clock::now();

                        pUI->template pushMsg<protocol_api::cmdPROGRESS>(protocol_api::SProgressCmd(
                            m_nofReceived,
                            m_nofRequests,
                            m_nofReceivedErrors,
                            std::chrono::duration_cast<std::chrono::milliseconds>(curTime - m_startTime).count()));
                    }

                    checkAllReceived();
                }
                catch (std::bad_weak_ptr& e)
                {
                    // TODO: Do we need to log something here?
                }

                return true;
            }

          private:
            void checkAllReceived()
            {
                try
                {
                    if (allReceived())
                    {
                        T* pThis = static_cast<T*>(this);
                        std::string userMessage = pThis->getAllReceivedMessage();

                        if (!m_channel.expired())
                        {
                            auto pUI = m_channel.lock();
                            pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(
                                protocol_api::SSimpleMsgCmd(userMessage, MiscCommon::info));
                            pUI->template pushMsg<protocol_api::cmdSHUTDOWN>();

                            m_channel.reset();
                        }
                    }
                }
                catch (std::bad_weak_ptr& e)
                {
                    // TODO: Do we need to log something here?
                }
            }

          public:
            size_t m_nofRequests;
            size_t m_nofReceived;
            size_t m_nofReceivedErrors;
            CAgentChannel::weakConnectionPtr_t m_channel;
            std::mutex m_mutexStart;
            std::mutex m_mutexReceive;

          private:
            std::chrono::steady_clock::time_point m_startTime;
        };

        class CGetLogChannelInfo : public CUIChannelInfo<CGetLogChannelInfo>
        {
          public:
            std::string getMessage(const protocol_api::SBinaryAttachmentReceivedCmd& _cmd,
                                   CAgentChannel::weakConnectionPtr_t _channel) const
            {
                std::stringstream ss;
                auto p = _channel.lock();
                ss << nofReceived() << "/" << m_nofRequests << " [" << p->getId() << "] -> "
                   << _cmd.m_requestedFileName;
                return ss.str();
            }

            std::string getErrorMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                        CAgentChannel::weakConnectionPtr_t _channel) const
            {
                std::stringstream ss;
                auto p = _channel.lock();
                ss << nofReceived() << "/" << m_nofRequests << " Error [" << p->getId() << "]: " << _cmd.m_sMsg;
                return ss.str();
            }

            std::string getAllReceivedMessage() const
            {
                std::stringstream ss;
                ss << "total: " << m_nofRequests << ", received: " << nofReceived()
                   << ", errors: " << m_nofReceivedErrors;
                return ss.str();
            }
        };

        class CTestChannelInfo : public CUIChannelInfo<CTestChannelInfo>
        {
          public:
            CTestChannelInfo()
                : CUIChannelInfo<CTestChannelInfo>()
                , m_totalReceived(0)
                , m_totalTime(0)
            {
            }

            std::string getMessage(const protocol_api::SBinaryAttachmentReceivedCmd& _cmd,
                                   CAgentChannel::weakConnectionPtr_t _channel) const
            {
                std::stringstream ss;
                auto p = _channel.lock();
                float downloadTime = 0.000001 * _cmd.m_downloadTime; // micros->s
                float speed = (downloadTime != 0.) ? 0.001 * _cmd.m_receivedFileSize / downloadTime : 0;
                ss << nofReceived() << "/" << m_nofRequests << " [" << p->getId() << "]: " << _cmd.m_receivedFileSize
                   << " bytes in " << downloadTime << " s (" << speed << " KB/s)";
                return ss.str();
            }

            std::string getErrorMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                        CAgentChannel::weakConnectionPtr_t _channel) const
            {
                std::stringstream ss;
                auto p = _channel.lock();
                ss << nofReceived() << "/" << m_nofRequests << " Error [" << p->getId() << "]: " << _cmd.m_sMsg;
                return ss.str();
            }

            std::string getAllReceivedMessage() const
            {
                std::stringstream ss;
                ss << "received: " << nofReceived() << ", total: " << m_nofRequests
                   << ", errors: " << m_nofReceivedErrors << " | ";

                float downloadTime = 0.000001 * m_totalTime; // micros->s
                float speed = (downloadTime != 0.) ? 0.001 * m_totalReceived / downloadTime : 0;
                ss << "download " << m_totalReceived << " bytes in " << downloadTime << " s (" << speed << " KB/s)";
                return ss.str();
            }

            size_t m_totalReceived; // [bytes]
            size_t m_totalTime;     // [ms]
        };

        class CActivateAgentsChannelInfo : public CUIChannelInfo<CActivateAgentsChannelInfo>
        {
          public:
            std::string getMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                   CAgentChannel::weakConnectionPtr_t _channel) const
            {
                std::stringstream ss;
                auto p = _channel.lock();
                ss << nofReceived() << "/" << m_nofRequests << " [" << p->getId() << "] -> Activated";
                return ss.str();
            }

            std::string getErrorMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                        CAgentChannel::weakConnectionPtr_t _channel) const
            {
                std::stringstream ss;
                auto p = _channel.lock();
                ss << nofReceived() << "/" << m_nofRequests << " Error [" << p->getId() << "]: " << _cmd.m_sMsg;
                return ss.str();
            }

            std::string getAllReceivedMessage() const
            {
                std::stringstream ss;
                ss << "total: " << m_nofRequests << ", activations: " << nofReceived()
                   << ", errors: " << m_nofReceivedErrors << "\n";

                return ss.str();
            }
        };

        class CStopUserTasksChannelInfo : public CUIChannelInfo<CStopUserTasksChannelInfo>
        {
          public:
            std::string getMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                   CAgentChannel::weakConnectionPtr_t _channel) const
            {
                std::stringstream ss;
                auto p = _channel.lock();
                ss << nofReceived() << "/" << m_nofRequests << " [" << p->getId() << "] -> Stopped";
                return ss.str();
            }

            std::string getErrorMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                        CAgentChannel::weakConnectionPtr_t _channel) const
            {
                std::stringstream ss;
                auto p = _channel.lock();
                ss << nofReceived() << "/" << m_nofRequests << " Error [" << p->getId() << "]: " << _cmd.m_sMsg;
                return ss.str();
            }

            std::string getAllReceivedMessage() const
            {
                std::stringstream ss;
                ss << "total: " << m_nofRequests << ", stopped: " << nofReceived()
                   << ", errors: " << m_nofReceivedErrors;
                return ss.str();
            }
        };

        class CSubmitAgentsChannelInfo : public CUIChannelInfo<CSubmitAgentsChannelInfo>
        {
          public:
            CSubmitAgentsChannelInfo()
                : CUIChannelInfo<CSubmitAgentsChannelInfo>()
                , m_PluginStartTime(std::chrono::system_clock::duration::zero())
                , m_bInit(false)
            {
            }

          public:
            std::string getMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                   CAgentChannel::weakConnectionPtr_t _channel) const
            {
                std::stringstream ss;
                auto p = _channel.lock();
                ss << _cmd.m_sMsg;
                return ss.str();
            }

            std::string getErrorMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                        CAgentChannel::weakConnectionPtr_t _channel) const
            {
                std::stringstream ss;
                auto p = _channel.lock();
                ss << _cmd.m_sMsg;
                return ss.str();
            }

            std::string getAllReceivedMessage() const
            {
                std::stringstream ss;
                // ss << "total: " << m_nofRequests << ", activations: " << nofReceived()
                // << ", errors: " << m_nofReceivedErrors << "\n";

                return ss.str();
            }

            bool processCustomCommandMessage(const protocol_api::SCustomCmdCmd& _cmd,
                                             CAgentChannel::weakConnectionPtr_t _channel)
            {
                LOG(MiscCommon::info) << "Processing RMS plug-in message...";
                boost::property_tree::ptree pt;

                try
                {
                    std::lock_guard<std::mutex> lock(m_mutexReceive);

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

                    std::istringstream ss(_cmd.m_sCmd);
                    boost::property_tree::read_json(ss, pt);

                    const boost::property_tree::ptree& childPT = pt.get_child("dds.plug-in");

                    for (const auto& child : childPT)
                    {
                        const std::string& tag = child.first;
                        if (tag == "init")
                        {
                            auto now = std::chrono::system_clock::now().time_since_epoch();
                            auto diff = now - m_PluginStartTime;
                            std::stringstream ssMsg;
                            ssMsg << "RMS plug-in is online. Startup time: "
                                  << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << "ms.";
                            LOG(MiscCommon::info) << ssMsg.str();
                            pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(
                                protocol_api::SSimpleMsgCmd(ssMsg.str(), MiscCommon::info));

                            SInit init;
                            init.fromPT(pt);
                            // Sending Submit request to the plug-in
                            protocol_api::SCustomCmdCmd cmd;
                            cmd.m_sCmd = m_strInitialSubmitRequest;
                            cmd.m_sCondition = "";
                            pPlugin->template pushMsg<protocol_api::cmdCUSTOM_CMD>(cmd);

                            // Subscribe on plug-in disconnect
                            pPlugin->subscribeOnEvent(protocol_api::EChannelEvents::OnRemoteEndDissconnected,
                                                      [this](CAgentChannel* _channel) {
                                                          // the plug-in is done and went offline, let's close UI
                                                          // connection as well.
                                                          m_channelSubmitPlugin.reset();
                                                          shutdown();
                                                      });
                        }
                        else if (tag == "message")
                        {
                            SMessage message;
                            message.fromPT(pt);
                            std::stringstream ss;
                            ss << "Plug-in: " << message.m_msg;
                            pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(protocol_api::SSimpleMsgCmd(
                                ss.str(),
                                (message.m_msgSeverity == EMsgSeverity::info ? MiscCommon::info : MiscCommon::error)));
                            if (message.m_msgSeverity == EMsgSeverity::error)
                            {
                                pUI->template pushMsg<protocol_api::cmdSHUTDOWN>();
                                m_channel.reset();
                            }
                        }
                    }
                }
                // catch explicetly ptree_error. On some systems the exact exception message is not propogated to
                // std::exception parent and can be lost
                catch (const boost::property_tree::ptree_error& _e)
                {
                    std::string msg("Failed to process RMS plug-in message: ");
                    msg += _e.what();
                    LOG(MiscCommon::error) << msg;

                    auto pUI = m_channel.lock();
                    if (pUI)
                        pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(
                            protocol_api::SSimpleMsgCmd(msg, MiscCommon::error));

                    shutdown();
                }
                catch (const std::exception& _e)
                {
                    LOG(MiscCommon::error) << "CSubmitAgentsChannelInfo::processCustomCommandMessage: ";
                    shutdown();
                }
                return true;
            }

            bool isPluginOnline()
            {
                return (!m_channel.expired());
            }

            // This function can be called to check whether plug-in failed to start.
            // If this is the case, the function will alert user and close connection to UI.
            void checkPluginFailedToStart()
            {
                std::lock_guard<std::mutex> lock(m_mutexReceive);
                // plug-in has not been yet started
                if (std::chrono::duration_cast<std::chrono::milliseconds>(m_PluginStartTime) ==
                        std::chrono::system_clock::duration::zero() &&
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
                    m_PluginStartTime = std::chrono::system_clock::duration::zero();
                    return;
                }

                // Give plug-in 2 minute to start
                auto now = std::chrono::system_clock::now().time_since_epoch();
                auto diff = now - m_PluginStartTime;
                if (diff >= std::chrono::minutes(2))
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
                        std::stringstream ss;
                        ss << "still waiting for plug-in (elapsed "
                           << std::chrono::duration_cast<std::chrono::seconds>(diff).count() << "s)...";
                        auto pUI = m_channel.lock();
                        if (pUI)
                            pUI->template pushMsg<protocol_api::cmdSIMPLE_MSG>(
                                protocol_api::SSimpleMsgCmd(ss.str(), MiscCommon::info));
                    }
                }
            }

            void shutdown()
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

            void initPlugin()
            {
                m_bInit = true;
                m_PluginStartTime = std::chrono::system_clock::now().time_since_epoch();
            }

          public:
            CAgentChannel::weakConnectionPtr_t m_channelSubmitPlugin; // pointer to the plug-in
            std::string m_strInitialSubmitRequest;
            std::chrono::system_clock::duration m_PluginStartTime;
            bool m_bInit;
        };
    }
}

#endif /* defined(__DDS__UIChannelInfo__) */
