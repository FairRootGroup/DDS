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
        //    void readHandler(const boost::system::error_code& _ec, size_t _bytesTransferred);
        //    void writeHandler(const boost::system::error_code& _ec, size_t bytesTransferred);
        //    size_t readCompleteHandler(const boost::system::error_code& _ec, size_t _bytesTransferred);
        //    void pingRequest();
        //    void checkPingHandler();
        //    void checkPing();
        //    void doRead();
        //    void doWrite(const std::string& msg);
        void readHeader();
        void readBody();

      private:
        boost::asio::ip::tcp::socket m_socket;
        //    boost::asio::deadline_timer m_timer;
        //    boost::posix_time::ptime m_lastPing;
        bool m_started;
        //
        //    enum
        //    {
        //        max_msg = 1024
        //    };
        //    char m_readBuffer[max_msg];
        //    char m_writeBuffer[max_msg];
        CProtocolMessage m_currentMsg;
        // messageQueue_t m_readMsgBuffer;
    };
}
#endif /* defined(__DDS__TalkToAgent__) */
