// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TestConnectionManager__
#define __DDS__TestConnectionManager__
// DDS
#include "ConnectionManagerImpl.h"
#include "TestChannel.h"

namespace dds
{
    class CTestConnectionManager : public CConnectionManagerImpl<CTestChannel, CTestConnectionManager>
    {
      public:
        CTestConnectionManager(const SOptions_t& _options,
                               boost::asio::io_service& _io_service,
                               boost::asio::ip::tcp::endpoint& _endpoint);

        ~CTestConnectionManager();

      public:
        void newClientCreated(CTestChannel::connectionPtr_t _newClient);
    };
}
#endif /* defined(__DDS__TestConnectionManager__) */
