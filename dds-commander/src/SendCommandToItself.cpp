// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SendCommandToItself.h"
#include "Logger.h"
// BOOST
#include "boost/asio.hpp"

using namespace MiscCommon;
using namespace std;

CSendCommandToItself::CSendCommandToItself(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator)
    : io_service_(io_service)
    , socket_(io_service)
{
    boost::asio::async_connect(socket_, endpoint_iterator, boost::bind(&chat_client::handle_connect, this, boost::asio::placeholders::error));
}
