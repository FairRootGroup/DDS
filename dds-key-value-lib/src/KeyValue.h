// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef KEYVALUE_H_
#define KEYVALUE_H_
// STD
#include <string>
#include <map>
#include <vector>
#include <chrono>
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
        void unsubscribe();

        void subscribeError(errorSignal_t::slot_function_type _subscriber);
        void unsubscribeError();
    };
}

#endif /* KEYVALUE_H_ */
