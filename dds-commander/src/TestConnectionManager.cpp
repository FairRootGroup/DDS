// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "TestConnectionManager.h"

using namespace dds;

CTestConnectionManager::CTestConnectionManager(const SOptions_t& _options,
                                               boost::asio::io_service& _io_service,
                                               boost::asio::ip::tcp::endpoint& _endpoint)
    : CConnectionManagerImpl<CTestChannel, CTestConnectionManager>(_options, _io_service, _endpoint)
{
}

CTestConnectionManager::~CTestConnectionManager()
{
}

void CTestConnectionManager::newClientCreated(CTestChannel::connectionPtr_t _newClient)
{
}