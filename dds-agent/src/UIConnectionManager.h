// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_UIConnectionManager_h
#define DDS_UIConnectionManager_h

// DDS
#include "ConnectionManagerImpl.h"
#include "UIChannel.h"
// BOOST
#include <boost/asio.hpp>

namespace dds
{
    class CUIConnectionManager : public CConnectionManagerImpl<CUIChannel, CUIConnectionManager>
    {
      public:
        CUIConnectionManager(boost::asio::io_service& _io_service, boost::asio::ip::tcp::endpoint& _endpoint);
        virtual ~CUIConnectionManager();

      public:
        void newClientCreated(CUIChannel::connectionPtr_t _newClient)
        {
        }
        void _start()
        {
        }
        void _stop()
        {
        }
        void _createInfoFile(size_t _port) const;
        void _deleteInfoFile() const;
    };
}

#endif
