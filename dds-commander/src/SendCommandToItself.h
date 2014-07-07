// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__SendCommandToItself__
#define __DDS__SendCommandToItself__

#include "boost/noncopyable.hpp"
#include "boost/asio.hpp"

class CSendCommandToItself
{
  public:
    CSendCommandToItself(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator);

  private:
    boost::asio::io_service& io_service_;
    tcp::socket socket_;
    // chat_message read_msg_;
    // chat_message_queue write_msgs_;
};

#endif /* defined(__DDS__TalkToAgent__) */
