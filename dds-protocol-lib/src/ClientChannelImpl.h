// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_ClientChannelImpl_h
#define DDS_ClientChannelImpl_h

// DDS
#include "BaseChannelImpl.h"
#include "version.h"
// STD
#include <functional>

namespace dds
{
    namespace protocol_api
    {
        template <class T>
        class CClientChannelImpl : public CBaseChannelImpl<T>
        {

            typedef std::function<void(T*)> handlerEventFunction_t;

          protected:
            CClientChannelImpl<T>(boost::asio::io_service& _service,
                                  EChannelType _channelType,
                                  uint64_t _protocolHeaderID)
                : CBaseChannelImpl<T>(_service, _protocolHeaderID)
            {
                this->m_channelType = _channelType;
                // Register handshake OK callback
                this->template registerHandler<cmdREPLY_HANDSHAKE_OK>(
                    [this](const SSenderInfo& _sender,
                           SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment) {
                        LOG(MiscCommon::info) << "Successfull handshake";

                        this->m_isHandshakeOK = true;

                        // The following commands starts message processing which might be queued before.
                        this->template pushMsg<cmdUNKNOWN>(_sender.m_ID);

                        // notify all subscribers about the event
                        this->dispatchHandlers(EChannelEvents::OnHandshakeOK, _sender);
                    });

                // Register handshake ERROR callback
                this->template registerHandler<cmdREPLY_HANDSHAKE_ERR>(
                    [this](const SSenderInfo& _sender,
                           SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_ERR>::ptr_t _attachment) {
                        LOG(MiscCommon::info) << "Handshake failed with the following error: " << _attachment->m_sMsg;

                        this->m_isHandshakeOK = false;
                        this->m_channelType = EChannelType::UNKNOWN;

                        // notify all subscribers about the event
                        this->dispatchHandlers(EChannelEvents::OnHandshakeFailed, _sender);

                        // close connection
                        this->stop();
                    });
            }

            ~CClientChannelImpl<T>()
            {
            }

          public:
            void reconnect()
            {
                connect(m_endpoint_iterator);
            }

            void connect(boost::asio::ip::tcp::resolver::iterator _endpoint_iterator)
            {
                m_endpoint_iterator = _endpoint_iterator;
                boost::asio::async_connect(
                    this->socket(),
                    _endpoint_iterator,
                    [this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator) {
                        if (!ec)
                        {
                            LOG(MiscCommon::debug) << "Client channel connected.";
                            // notify all subscribers about the event
                            this->dispatchHandlers(EChannelEvents::OnConnected, SSenderInfo());

                            // start the communication channel
                            this->start();

                            // Prepare a hand shake message
                            SVersionCmd cmd;
                            cmd.m_channelType = this->m_channelType;
                            cmd.m_sSID = this->m_sessionID;
                            cmd.m_version = DDS_PROTOCOL_VERSION;
                            this->template pushMsg<cmdHANDSHAKE>(cmd, this->m_protocolHeaderID);
                        }
                        else
                        {
                            LOG(MiscCommon::error) << "Failed to connect to remote end.";
                            // notify all subscribers about the event
                            this->dispatchHandlers(EChannelEvents::OnFailedToConnect, SSenderInfo());
                        }
                    });
            }

          private:
            boost::asio::ip::tcp::resolver::iterator m_endpoint_iterator;
        };
    } // namespace protocol_api
} // namespace dds

#endif
