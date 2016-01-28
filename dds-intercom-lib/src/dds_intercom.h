// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_INTERCOM_H_
#define DDS_INTERCOM_H_
// STD
#include <string>
// BOOST
#include <boost/signals2/signal.hpp>

namespace dds
{
    class CKeyValue
    {
      public:
        typedef std::map<std::string, std::string> valuesMap_t;
        typedef boost::signals2::signal<void(const std::string&, const std::string&)> signal_t;
        typedef boost::signals2::signal<void(const std::string&)> errorSignal_t;
        typedef boost::signals2::connection connection_t;

      public:
        ~CKeyValue();

      public:
        int putValue(const std::string& _key, const std::string& _value);
        void getValues(const std::string& _key, valuesMap_t* _values);
        void subscribe(signal_t::slot_function_type _subscriber);
        void subscribeError(errorSignal_t::slot_function_type _subscriber);
        void unsubscribe();
    };

    class CCustomCmd
    {
      public:
        typedef boost::signals2::signal<void(const std::string&, const std::string&, uint64_t)> signal_t;
        typedef boost::signals2::signal<void(const std::string&)> replySignal_t;
        typedef boost::signals2::connection connection_t;

      public:
        ~CCustomCmd();

      public:
        int send(const std::string& _command, const std::string& _condition);
        void subscribe(signal_t::slot_function_type _subscriber);
        void subscribeReply(replySignal_t::slot_function_type _subscriber);
        void unsubscribe();
    };
}

#endif /* DDS_INTERCOM_H_ */
