// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_ServerChannelImpl_h
#define DDS_ServerChannelImpl_h

#include "BaseChannelImpl.h"

namespace dds
{
    namespace protocol_api
    {
        template <class T>
        class CServerChannelImpl : public CBaseChannelImpl<T>
        {
          protected:
            CServerChannelImpl<T>(boost::asio::io_service& _service, const channelTypeVector_t _requiredChannelTypes)
                : CBaseChannelImpl<T>(_service)
                , m_requiredChannelTypes(_requiredChannelTypes)
            {
                // Register handshake callback
                std::function<void(SCommandAttachmentImpl<cmdHANDSHAKE>::ptr_t)> funcHandshake =
                    [this](SCommandAttachmentImpl<cmdHANDSHAKE>::ptr_t _attachment) {
                        // send shutdown if versions are incompatible
                        bool versionCompatible = m_requiredChannelTypes.empty();

                        if (!versionCompatible)
                        {
                            for (const auto& v : m_requiredChannelTypes)
                            {
                                SVersionCmd versionCmd;
                                versionCmd.m_channelType = v;
                                versionCompatible = (*_attachment == versionCmd);
                                if (versionCompatible)
                                    break;
                            }
                        }

                        if (!versionCompatible)
                        {
                            this->m_isHandshakeOK = false;
                            this->m_channelType = EChannelType::UNKNOWN;
                            // Send reply that the version of the protocol is incompatible
                            std::string msg("Incompatible protocol version of the client");
                            LOG(MiscCommon::warning) << msg << this->remoteEndIDString();
                            this->template pushMsg<cmdREPLY_HANDSHAKE_ERR>(SSimpleMsgCmd(msg, MiscCommon::fatal));

                            // notify all subscribers about the event
                            this->onEvent(EChannelEvents::OnHandshakeFailed);
                        }
                        else
                        {
                            this->m_isHandshakeOK = true;
                            this->m_channelType = static_cast<EChannelType>(_attachment->m_channelType);

                            // The following commands starts message processing which might have been queued before.
                            this->template pushMsg<cmdUNKNOWN>();

                            // everything is OK, we can work with this agent
                            LOG(MiscCommon::info) << "[" << this->socket().remote_endpoint().address().to_string()
                                                  << "] has successfully connected.";

                            this->template pushMsg<cmdREPLY_HANDSHAKE_OK>();

                            // notify all subscribers about the event
                            this->onEvent(EChannelEvents::OnHandshakeOK);
                        }
                    };
                this->template registerHandler<cmdHANDSHAKE>(funcHandshake);
            }

            ~CServerChannelImpl<T>()
            {
            }

          private:
            channelTypeVector_t m_requiredChannelTypes;
        };
    }
}

#endif
