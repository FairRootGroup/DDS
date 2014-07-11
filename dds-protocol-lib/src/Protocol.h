// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef PROTOCOL_H_
#define PROTOCOL_H_
// DDS
#include "ProtocolMessage.h"
// MiscCommon
#include "def.h"
// STD
#include <deque>
#include <memory>
// BOOST
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

namespace dds
{
    //----------------------------------------------------------------------

    typedef std::deque<CProtocolMessage> messageQueue_t;

    //----------------------------------------------------------------------

    class session_participant
    {
      public:
        virtual ~session_participant()
        {
        }
        virtual void deliver(const CProtocolMessage& msg) = 0;
    };

    typedef std::shared_ptr<session_participant> sessionParticipantPtr_t;

    //----------------------------------------------------------------------

    class server_room
    {
      public:
        void join(sessionParticipantPtr_t participant)
        {
            participants_.insert(participant);
            for (auto msg : recent_msgs_)
                participant->deliver(msg);
        }

        void leave(sessionParticipantPtr_t participant)
        {
            participants_.erase(participant);
        }

        void deliver(const CProtocolMessage& msg)
        {
            recent_msgs_.push_back(msg);
            while (recent_msgs_.size() > max_recent_msgs)
                recent_msgs_.pop_front();

            for (auto participant : participants_)
                participant->deliver(msg);
        }

      private:
        std::set<sessionParticipantPtr_t> participants_;
        enum
        {
            max_recent_msgs = 100
        };
        messageQueue_t recent_msgs_;
    };

    //----------------------------------------------------------------------

    class server_session : public session_participant, public std::enable_shared_from_this<server_session>
    {
      public:
        server_session(tcp::socket socket, server_room& room)
            : socket_(std::move(socket))
            , room_(room)
        {
        }

        void start()
        {
            room_.join(shared_from_this());
            do_read_header();
        }

        void deliver(const CProtocolMessage& msg)
        {
            bool write_in_progress = !write_msgs_.empty();
            write_msgs_.push_back(msg);
            if (!write_in_progress)
            {
                do_write();
            }
        }

      private:
        void do_read_header()
        {
            auto self(shared_from_this());
            boost::asio::async_read(socket_,
                                    boost::asio::buffer(read_msg_.data(), CProtocolMessage::header_length),
                                    [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                    {
                if (!ec && read_msg_.decode_header())
                {
                    do_read_body();
                }
                else
                {
                    room_.leave(shared_from_this());
                }
            });
        }

        void do_read_body()
        {
            auto self(shared_from_this());
            boost::asio::async_read(socket_,
                                    boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                    [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                    {
                if (!ec)
                {
                    room_.deliver(read_msg_);
                    do_read_header();
                }
                else
                {
                    room_.leave(shared_from_this());
                }
            });
        }

        void do_write()
        {
            auto self(shared_from_this());
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
                                     [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                     {
                if (!ec)
                {
                    write_msgs_.pop_front();
                    if (!write_msgs_.empty())
                    {
                        do_write();
                    }
                }
                else
                {
                    room_.leave(shared_from_this());
                }
            });
        }

        tcp::socket socket_;
        server_room& room_;
        CProtocolMessage read_msg_;
        messageQueue_t write_msgs_;
    };

    //----------------------------------------------------------------------

    class protocol_server
    {
      public:
        protocol_server(boost::asio::io_service& io_service, const tcp::endpoint& endpoint)
            : acceptor_(io_service, endpoint)
            , socket_(io_service)
        {
            do_accept();
        }

      private:
        void do_accept()
        {
            acceptor_.async_accept(socket_,
                                   [this](boost::system::error_code ec)
                                   {
                if (!ec)
                {
                    std::make_shared<server_session>(std::move(socket_), room_)->start();
                }

                do_accept();
            });
        }

        tcp::acceptor acceptor_;
        tcp::socket socket_;
        server_room room_;
    };

    //----------------------------------------------------------------------
}

#endif /* PROTOCOL_H_ */
