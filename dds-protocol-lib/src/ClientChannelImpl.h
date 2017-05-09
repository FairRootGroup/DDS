// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_ClientChannelImpl_h
#define DDS_ClientChannelImpl_h

#include "BaseChannelImpl.h"
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
            CClientChannelImpl<T>(boost::asio::io_service& _service, EChannelType _channelType)
                : CBaseChannelImpl<T>(_service)
            {
                this->m_channelType = _channelType;
                // Register handshake OK callback
                std::function<void(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t)> funcHandshakeOK =
                    [this](SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment) {
                        LOG(MiscCommon::info) << "Successfull handshake";

                        this->m_isHandshakeOK = true;

                        // The following commands starts message processing which might be queued before.
                        this->template pushMsg<cmdUNKNOWN>();

                        // notify all subscribers about the event
                        this->dispatchHandlers(EChannelEvents::OnHandshakeOK);
                    };
                this->template registerHandler<cmdREPLY_HANDSHAKE_OK>(funcHandshakeOK);

                // Register handshake ERROR callback
                std::function<void(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_ERR>::ptr_t)> funcHandshakeERR =
                    [this](SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_ERR>::ptr_t _attachment) {
                        LOG(MiscCommon::info) << "Handshake failed with the following error: " << _attachment->m_sMsg;

                        this->m_isHandshakeOK = false;
                        this->m_channelType = EChannelType::UNKNOWN;

                        // notify all subscribers about the event
                        this->dispatchHandlers(EChannelEvents::OnHandshakeFailed);
                    };
                this->template registerHandler<cmdREPLY_HANDSHAKE_ERR>(funcHandshakeERR);
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
                            this->dispatchHandlers(EChannelEvents::OnConnected);

                            // start the communication channel
                            this->start();

                            // Prepare a hand shake message
                            SVersionCmd cmd;
                            cmd.m_channelType = this->m_channelType;
                            this->template pushMsg<cmdHANDSHAKE>(cmd);
                        }
                        else
                        {
                            LOG(MiscCommon::error) << "Failed to connect to remote end.";
                            // notify all subscribers about the event
                            this->dispatchHandlers(EChannelEvents::OnFailedToConnect);
                        }
                    });
            }

          private:
            boost::asio::ip::tcp::resolver::iterator m_endpoint_iterator;
        };
    }
}

#endif
