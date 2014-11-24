// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef KEYVALUE_H_
#define KEYVALUE_H_
// STD
#include <string>
#include <map>
#include <chrono>

namespace dds
{
    class CKeyValue
    {
      public:
        typedef std::map<uint64_t, std::string> container_t;

      public:
        void putValue(const std::string& _key, const std::string& _value);
        void getValue(const std::string& _key,
                      container_t* _values,
                      const std::chrono::system_clock::duration &_timeout = std::chrono::system_clock::duration::zero());
    };
}

#endif /* KEYVALUE_H_ */
