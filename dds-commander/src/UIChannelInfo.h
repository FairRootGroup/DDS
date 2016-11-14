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
                , m_shutdownOnComplete(true)
                , m_srcCommand(0)
                , m_startTime(std::chrono::steady_clock::now())
            {
            }

          public:
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
                            m_srcCommand,
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
                            m_srcCommand,
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

                            if (m_shutdownOnComplete)
                            {
                                pUI->template pushMsg<protocol_api::cmdSHUTDOWN>();
                            }
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
            bool m_shutdownOnComplete;
            uint16_t m_srcCommand;

          private:
            std::chrono::steady_clock::time_point m_startTime;
        };

        class CGetLogChannelInfo : public CUIChannelInfo<CGetLogChannelInfo>
        {
          public:
            CGetLogChannelInfo()
                : CUIChannelInfo<CGetLogChannelInfo>()
            {
                m_srcCommand = protocol_api::cmdGET_LOG;
            }

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
                m_srcCommand = protocol_api::cmdTRANSPORT_TEST;
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

        class CUpdateTopologyChannelInfo : public CUIChannelInfo<CUpdateTopologyChannelInfo>
        {
          public:
            CUpdateTopologyChannelInfo()
                : CUIChannelInfo<CUpdateTopologyChannelInfo>()
            {
                m_srcCommand = protocol_api::cmdACTIVATE_AGENT;
            }

            std::string getMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                   CAgentChannel::weakConnectionPtr_t _channel) const
            {
                std::stringstream ss;
                auto p = _channel.lock();
                std::string str = (m_srcCommand == protocol_api::cmdACTIVATE_AGENT) ? "Activated" : "Stopped";
                ss << nofReceived() << "/" << m_nofRequests << " [" << p->getId() << "] -> " << str;
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
                std::string str = (m_srcCommand == protocol_api::cmdACTIVATE_AGENT) ? "activations" : "stopped";
                ss << "total: " << m_nofRequests << ", " << str << ": " << nofReceived()
                   << ", errors: " << m_nofReceivedErrors << "\n";

                return ss.str();
            }
        };

        class CSubmitAgentsChannelInfo : public CUIChannelInfo<CSubmitAgentsChannelInfo>
        {
          public:
            CSubmitAgentsChannelInfo();

          public:
            std::string getMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                   CAgentChannel::weakConnectionPtr_t _channel) const;
            std::string getErrorMessage(const protocol_api::SSimpleMsgCmd& _cmd,
                                        CAgentChannel::weakConnectionPtr_t _channel) const;
            std::string getAllReceivedMessage() const;
            bool processCustomCommandMessage(const protocol_api::SCustomCmdCmd& _cmd,
                                             CAgentChannel::weakConnectionPtr_t _channel);
            bool isPluginOnline();
            // This function can be called to check whether plug-in failed to start.
            // If this is the case, the function will alert user and close connection to UI.
            void checkPluginFailedToStart();
            void shutdown();
            void initPlugin();

          public:
            CAgentChannel::weakConnectionPtr_t m_channelSubmitPlugin; // pointer to the plug-in
            std::string m_strInitialSubmitRequest;
            std::chrono::system_clock::duration m_PluginStartTime;
            bool m_bInit;
        };
    }
}

#endif /* defined(__DDS__UIChannelInfo__) */
