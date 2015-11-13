// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef CUSTOMCMD_H_
#define CUSTOMCMD_H_
// STD
#include <string>
// BOOST
#include <boost/signals2/signal.hpp>

namespace dds
{
    namespace custom_cmd
    {
        class CCustomCmd
        {
          public:
            typedef boost::signals2::signal<void(const std::string&, const std::string&, uint64_t)> cmdSignal_t;
            typedef boost::signals2::signal<void(const std::string&)> replySignal_t;
            typedef boost::signals2::connection connection_t;

          public:
            ~CCustomCmd();

          public:
            int sendCmd(const std::string& _command, const std::string& _condition);
            void subscribeCmd(cmdSignal_t::slot_function_type _subscriber);
            void subscribeReply(replySignal_t::slot_function_type _subscriber);
            void unsubscribe();
        };
    }
}

#endif /* CUSTOMCMD_H_ */
