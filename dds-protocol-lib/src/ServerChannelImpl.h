// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_ServerChannelImpl_h
#define DDS_ServerChannelImpl_h

// DDS
#include "BaseChannelImpl.h"
#include "version.h"

namespace dds
{
    namespace protocol_api
    {
        template <class T>
        class CServerChannelImpl : public CBaseChannelImpl<T>
        {
          protected:
            CServerChannelImpl<T>(boost::asio::io_service& _service, const channelTypeVector_t _requiredChannelTypes)
                : CBaseChannelImpl<T>(_service, 0)
                , m_requiredChannelTypes(_requiredChannelTypes)
            {
                // Register handshake callback
                this->template registerHandler<cmdHANDSHAKE>(
                    [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdHANDSHAKE>::ptr_t _attachment) {
                        // Check that the client channel is actually supported
                        bool isSupportedChnl(false);
                        for (const auto& v : m_requiredChannelTypes)
                        {
                            isSupportedChnl = (_attachment->m_channelType == v);
                            if (isSupportedChnl)
                                break;
                        }

                        if (!isSupportedChnl)
                        {
                            HandshakeFailed("Unsupported channel type");
                            return;
                        }

                        // send shutdown if versions are incompatible
                        if (_attachment->m_version != DDS_PROTOCOL_VERSION)
                        {
                            std::stringstream ss;
                            ss << "Incompatible protocol version. Server: " << DDS_PROTOCOL_VERSION
                               << " Client: " << _attachment->m_version;
                            HandshakeFailed(ss.str());
                            return;
                        }

                        // Check Session ID
                        if (this->m_sessionID.empty() || this->m_sessionID != _attachment->m_sSID)
                        {
                            HandshakeFailed("Incompatible Session ID");
                            return;
                        }

                        this->m_isHandshakeOK = true;
                        this->m_channelType = static_cast<EChannelType>(_attachment->m_channelType);

                        // The following commands starts message processing which might have been queued before.
                        this->template pushMsg<cmdUNKNOWN>();

                        // everything is OK, we can work with this agent
                        LOG(MiscCommon::info) << "[" << this->socket().remote_endpoint().address().to_string()
                                              << "] has successfully connected.";

                        this->template pushMsg<cmdREPLY_HANDSHAKE_OK>();

                        // notify all subscribers about the event
                        this->dispatchHandlers(EChannelEvents::OnHandshakeOK, _sender);
                    });
            }

            ~CServerChannelImpl<T>()
            {
            }

            void HandshakeFailed(const std::string& _reason)
            {
                this->m_isHandshakeOK = false;
                this->m_channelType = EChannelType::UNKNOWN;
                LOG(MiscCommon::warning) << _reason << "; Client: " << this->remoteEndIDString();
                this->template pushMsg<cmdREPLY_HANDSHAKE_ERR>(SSimpleMsgCmd(_reason, MiscCommon::fatal));

                // notify all subscribers about the event
                this->dispatchHandlers(EChannelEvents::OnHandshakeFailed, SSenderInfo());
            }

          private:
            channelTypeVector_t m_requiredChannelTypes;
        };
    }
}

#endif
