// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__CommanderServer__
#define __DDS__CommanderServer__

// BOOST
#include <boost/asio.hpp>

class CCommanderServer
{
  public:
    CCommanderServer();

    virtual ~CCommanderServer();

    void start();

    void stop();

  private:
    boost::asio::io_service m_service;
};

#endif /* defined(__DDS__CommanderServer__) */
