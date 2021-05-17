// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_ServerChannelImpl_h
#define DDS_ServerChannelImpl_h

// DDS
#include "BaseChannelImpl.h"
#include "Version.h"

namespace dds
{
    namespace protocol_api
    {
        template <class T>
        class CServerChannelImpl : public CBaseChannelImpl<T>
        {
          protected:
            CServerChannelImpl<T>(boost::asio::io_context& _service, const channelTypeVector_t _requiredChannelTypes)
                : CBaseChannelImpl<T>(_service, 0)
                , m_requiredChannelTypes(_requiredChannelTypes)
            {
                // Register handshake callback
                this->template registerHandler<cmdHANDSHAKE>(
                    [this](const SSenderInfo& _sender, SCommandAttachmentImpl<cmdHANDSHAKE>::ptr_t _attachment) {
                        if (!processHandshake_channelTypeSupported(
                                static_cast<EChannelType>(_attachment->m_channelType), true, _sender.m_ID))
                            return;
                        if (!processHandshake_versionMatch(_attachment->m_version, true, _sender.m_ID))
                            return;
                        if (!processHandshake_sessionIDMatch(_attachment->m_sSID, true, _sender.m_ID))
                            return;

                        this->m_protocolHeaderID = _sender.m_ID;
                        this->m_isHandshakeOK = true;
                        this->m_channelType = static_cast<EChannelType>(_attachment->m_channelType);

                        // The following commands starts message processing which might have been queued before.
                        this->template pushMsg<cmdUNKNOWN>();

                        // everything is OK, we can work with this agent
                        LOG(MiscCommon::info) << "[" << this->socket().remote_endpoint().address().to_string()
                                              << "] has successfully connected.";

                        this->template pushMsg<cmdREPLY_HANDSHAKE_OK>(_sender.m_ID);

                        // notify all subscribers about the event
                        this->dispatchHandlers(EChannelEvents::OnHandshakeOK, _sender);
                    });

                // Register handshake callback
                //                this->template registerHandler<cmdLOBBY_MEMBER_HANDSHAKE>(
                //                    [this](const SSenderInfo& _sender,
                //                           SCommandAttachmentImpl<cmdLOBBY_MEMBER_HANDSHAKE>::ptr_t _attachment) {
                //                        if (!processHandshake_channelTypeSupported(
                //                                static_cast<EChannelType>(_attachment->m_channelType), false,
                //                                _sender.m_ID))
                //                            return;
                //                        if (!processHandshake_versionMatch(_attachment->m_version, false,
                //                        _sender.m_ID))
                //                            return;
                //                        if (!processHandshake_sessionIDMatch(_attachment->m_sSID, false,
                //                        _sender.m_ID))
                //                            return;
                //
                //                        // everything is OK, we can work with this agent
                //                        LOG(MiscCommon::info)
                //                            << "New lobby member [" << _sender.m_ID << "] has successfully
                //                            connected.";
                //
                //                        SReplyCmd cmd =
                //                            SReplyCmd("", (uint16_t)SReplyCmd::EStatusCode::OK, 0,
                //                            (uint16_t)cmdLOBBY_MEMBER_HANDSHAKE);
                //                        this->template pushMsg<cmdREPLY>(cmd, _sender.m_ID);
                //
                //                        // notify all subscribers about the event
                //                        this->dispatchHandlers(EChannelEvents::OnLobbyMemberHandshakeOK, _sender);
                //                    });
            }

            ~CServerChannelImpl<T>()
            {
            }

          private:
            bool processHandshake_channelTypeSupported(EChannelType _channelType, bool _lobbyLeader, uint64_t _senderID)
            {
                // Check that the client channel is actually supported
                bool isSupportedChnl(false);
                for (const auto& v : m_requiredChannelTypes)
                {
                    isSupportedChnl = (_channelType == v);
                    if (isSupportedChnl)
                        break;
                }

                if (!isSupportedChnl)
                {
                    handshakeFailed("Unsupported channel type", _lobbyLeader, _senderID);
                    return false;
                }
                return true;
            }

            bool processHandshake_versionMatch(uint16_t _version, bool _lobbyLeader, uint64_t _senderID)
            {
                // send shutdown if versions are incompatible
                if (_version != DDS_PROTOCOL_VERSION)
                {
                    std::stringstream ss;
                    ss << "Incompatible protocol version. Server: " << DDS_PROTOCOL_VERSION << " Client: " << _version;
                    handshakeFailed(ss.str(), _lobbyLeader, _senderID);
                    return false;
                }
                return true;
            }

            bool processHandshake_sessionIDMatch(const std::string& _sessionID, bool _lobbyLeader, uint64_t _senderID)
            {
                // Check Session ID
                if (this->m_sessionID.empty() || this->m_sessionID != _sessionID)
                {
                    handshakeFailed("Incompatible Session ID", _lobbyLeader, _senderID);
                    return false;
                }
                return true;
            }

            void handshakeFailed(const std::string& _reason, bool /*_lobbyLeader*/, uint64_t _senderID)
            {
                this->m_isHandshakeOK = false;
                this->m_channelType = EChannelType::UNKNOWN;
                LOG(MiscCommon::warning) << _reason << "; Client: " << this->remoteEndIDString();

                this->template pushMsg<cmdREPLY_HANDSHAKE_ERR>(SSimpleMsgCmd(_reason, MiscCommon::fatal), _senderID);
                // notify all subscribers about the event
                this->dispatchHandlers(EChannelEvents::OnHandshakeFailed, SSenderInfo());
            }

          private:
            channelTypeVector_t m_requiredChannelTypes;
        };
    } // namespace protocol_api
} // namespace dds

#endif
