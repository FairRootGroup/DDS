// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__TalkToAgent__
#define __DDS__TalkToAgent__
// STD
#include <deque>
// BOOST
#include "boost/noncopyable.hpp"
#include "boost/asio.hpp"
// DDS
#include "ProtocolMessage.h"

namespace dds
{
    class CTalkToAgent;
    typedef std::shared_ptr<CTalkToAgent> TalkToAgentPtr_t;
    typedef std::vector<TalkToAgentPtr_t> TalkToAgentPtrVector_t;
    typedef std::deque<CProtocolMessage> messageQueue_t;

    class CTalkToAgent : public std::enable_shared_from_this<CTalkToAgent>, boost::noncopyable
    {
        CTalkToAgent(boost::asio::io_service& _service);

      public:
        static TalkToAgentPtr_t makeNew(boost::asio::io_service& _service);
        void start();
        void stop();

        boost::asio::ip::tcp::socket& socket();

      private:
        void readHeader();
        void readBody();
        void processMessage();

      private:
        boost::asio::ip::tcp::socket m_socket;
        bool m_started;
        CProtocolMessage m_currentMsg;
        // messageQueue_t m_readMsgBuffer;
    };
}
#endif /* defined(__DDS__TalkToAgent__) */
