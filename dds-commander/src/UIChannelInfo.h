// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__UIChannelInfo__
#define __DDS__UIChannelInfo__

// DDS
#include "AgentChannel.h"
#include "ProtocolCommands.h"
// STD
#include <mutex>
#include <string>
#include <sstream>

namespace dds
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
                    pUI->template pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(userMessage, MiscCommon::info));
                    pUI->template pushMsg<cmdPROGRESS>(SProgressCmd(m_nofReceived, m_nofRequests, m_nofReceivedErrors));
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
                    pUI->template pushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(userMessage, MiscCommon::error));
                    pUI->template pushMsg<cmdPROGRESS>(SProgressCmd(m_nofReceived, m_nofRequests, m_nofReceivedErrors));
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
                        pUI->template syncPushMsg<cmdSIMPLE_MSG>(SSimpleMsgCmd(userMessage, MiscCommon::info));
                        pUI->template pushMsg<cmdSHUTDOWN>();

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
    };

    class CGetLogChannelInfo : public CUIChannelInfo<CGetLogChannelInfo>
    {
      public:
        std::string getMessage(const SBinaryAttachmentReceivedCmd& _cmd,
                               CAgentChannel::weakConnectionPtr_t _channel) const
        {
            std::stringstream ss;
            auto p = _channel.lock();
            ss << nofReceived() << "/" << m_nofRequests << " [" << p->getId() << "] -> " << _cmd.m_requestedFileName;
            return ss.str();
        }

        std::string getErrorMessage(const SSimpleMsgCmd& _cmd, CAgentChannel::weakConnectionPtr_t _channel) const
        {
            std::stringstream ss;
            auto p = _channel.lock();
            ss << nofReceived() << "/" << m_nofRequests << " Error [" << p->getId() << "]: " << _cmd.m_sMsg;
            return ss.str();
        }

        std::string getAllReceivedMessage() const
        {
            std::stringstream ss;
            ss << "total: " << m_nofRequests << ", recieved: " << nofReceived() << ", errors: " << m_nofReceivedErrors;
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

        std::string getMessage(const SBinaryAttachmentReceivedCmd& _cmd,
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

        std::string getErrorMessage(const SSimpleMsgCmd& _cmd, CAgentChannel::weakConnectionPtr_t _channel) const
        {
            std::stringstream ss;
            auto p = _channel.lock();
            ss << nofReceived() << "/" << m_nofRequests << " Error [" << p->getId() << "]: " << _cmd.m_sMsg;
            return ss.str();
        }

        std::string getAllReceivedMessage() const
        {
            std::stringstream ss;
            ss << "recieved: " << nofReceived() << ", total: " << m_nofRequests << ", errors: " << m_nofReceivedErrors
               << " | ";

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
        std::string getMessage(const SSimpleMsgCmd& _cmd, CAgentChannel::weakConnectionPtr_t _channel) const
        {
            std::stringstream ss;
            auto p = _channel.lock();
            ss << nofReceived() << "/" << m_nofRequests << " [" << p->getId() << "] -> Activated";
            return ss.str();
        }

        std::string getErrorMessage(const SSimpleMsgCmd& _cmd, CAgentChannel::weakConnectionPtr_t _channel) const
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
               << ", errors: " << m_nofReceivedErrors;
            return ss.str();
        }
    };

    class CStopUserTasksChannelInfo : public CUIChannelInfo<CStopUserTasksChannelInfo>
    {
      public:
        std::string getMessage(const SSimpleMsgCmd& _cmd, CAgentChannel::weakConnectionPtr_t _channel) const
        {
            std::stringstream ss;
            auto p = _channel.lock();
            ss << nofReceived() << "/" << m_nofRequests << " [" << p->getId() << "] -> Stopped";
            return ss.str();
        }

        std::string getErrorMessage(const SSimpleMsgCmd& _cmd, CAgentChannel::weakConnectionPtr_t _channel) const
        {
            std::stringstream ss;
            auto p = _channel.lock();
            ss << nofReceived() << "/" << m_nofRequests << " Error [" << p->getId() << "]: " << _cmd.m_sMsg;
            return ss.str();
        }

        std::string getAllReceivedMessage() const
        {
            std::stringstream ss;
            ss << "total: " << m_nofRequests << ", stopped: " << nofReceived() << ", errors: " << m_nofReceivedErrors;
            return ss.str();
        }
    };
}

#endif /* defined(__DDS__UIChannelInfo__) */
