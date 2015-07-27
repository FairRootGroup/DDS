// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_UIConnectionManager_h
#define DDS_UIConnectionManager_h

// DDS
#include "ConnectionManagerImpl.h"
#include "UIChannel.h"
#include "CommanderChannel.h"
// BOOST
#include <boost/asio.hpp>

namespace dds
{
    namespace agent_cmd
    {
        class CUIConnectionManager : public protocol_api::CConnectionManagerImpl<CUIChannel, CUIConnectionManager>
        {
          public:
            CUIConnectionManager();
            virtual ~CUIConnectionManager();

          public:
            void newClientCreated(CUIChannel::connectionPtr_t _newClient);
            void _start()
            {
            }
            void _stop()
            {
            }
            void _createInfoFile(const std::vector<size_t>& _ports) const;
            void _deleteInfoFile() const;
            void setCommanderChannel(CCommanderChannel::weakConnectionPtr_t _channel)
            {
                m_commanderChannel = _channel;
            }
            void notifyAboutKeyUpdate(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);
            void notifyAboutSimpleMsg(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment);

          private:
            bool on_cmdUPDATE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment,
                                  CUIChannel::weakConnectionPtr_t _channel);

          private:
            CCommanderChannel::weakConnectionPtr_t m_commanderChannel;
        };
    }
}

#endif
