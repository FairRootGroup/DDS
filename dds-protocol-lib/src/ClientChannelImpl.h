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
            std::function<bool(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment,
                               CClientChannelImpl * _channel)> funcHandshakeOK =
                [this](SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_OK>::ptr_t _attachment, CClientChannelImpl* _channel)
                    -> bool
            {
                LOG(MiscCommon::info) << "Successfull handshake";

                this->m_isHandshakeOK = true;

                // The following commands starts message processing which might be queued before.
                this->template pushMsg<cmdUNKNOWN>();

                // give a chance child to execute something
                T* pThis = static_cast<T*>(this);
                pThis->onHandshakeOK();

                // Call external event handler
                if (m_handshakeOKEventHandler)
                    m_handshakeOKEventHandler(pThis);

                return true;
            };
            this->template registerMessageHandler<cmdREPLY_HANDSHAKE_OK>(funcHandshakeOK);

            // Register handshake ERROR callback
            std::function<bool(SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_ERR>::ptr_t _attachment,
                               CClientChannelImpl * _channel)> funcHandshakeERR =
                [this](SCommandAttachmentImpl<cmdREPLY_HANDSHAKE_ERR>::ptr_t _attachment, CClientChannelImpl* _channel)
                    -> bool
            {
                LOG(MiscCommon::info) << "Handshake failed with the following error: " << _attachment->m_sMsg;

                this->m_isHandshakeOK = false;
                this->m_channelType = EChannelType::UNKNOWN;

                // give a chance child to execute something
                T* pThis = static_cast<T*>(this);
                pThis->onHandshakeERR();

                // Call external event handler
                if (m_handshakeERREventHandler)
                    m_handshakeERREventHandler(pThis);

                return true;
            };
            this->template registerMessageHandler<cmdREPLY_HANDSHAKE_ERR>(funcHandshakeERR);
        }

        ~CClientChannelImpl<T>()
        {
        }

      public:
        void connect(boost::asio::ip::tcp::resolver::iterator _endpoint_iterator)
        {
            boost::asio::async_connect(this->socket(),
                                       _endpoint_iterator,
                                       [this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
                                       {
                                           if (!ec)
                                           {
                                               // give a chance child to execute something
                                               T* pThis = static_cast<T*>(this);
                                               pThis->onConnected();

                                               // Call external event handler
                                               if (m_connectEventHandler)
                                                   m_connectEventHandler(pThis);

                                               // start the communication channel
                                               this->start();

                                               // Prepare a hand shake message
                                               SVersionCmd cmd;
                                               cmd.m_channelType = this->m_channelType;
                                               this->template pushMsg<cmdHANDSHAKE>(cmd);
                                           }
                                           else
                                           {
                                               // give a chance child to execute something
                                               T* pThis = static_cast<T*>(this);
                                               pThis->onFailedToConnect();
                                           }
                                       });
        }

        void registerConnectEventHandler(handlerEventFunction_t _handler)
        {
            m_connectEventHandler = _handler;
        }

        void registerHandshakeOKEventHandler(handlerEventFunction_t _handler)
        {
            m_handshakeOKEventHandler = _handler;
        }

        void registerHandshakeERREventHandler(handlerEventFunction_t _handler)
        {
            m_handshakeOKEventHandler = _handler;
        }

      private:
        handlerEventFunction_t m_connectEventHandler;
        handlerEventFunction_t m_handshakeOKEventHandler;
        handlerEventFunction_t m_handshakeERREventHandler;
    };
}

#endif
