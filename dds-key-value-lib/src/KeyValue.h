// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef KEYVALUE_H_
#define KEYVALUE_H_
// STD
#include <string>
#include <map>
#include <set>
#include <chrono>

namespace dds
{
    const std::chrono::system_clock::duration g_maxWaitTime = std::chrono::seconds(60);
    class CKeyValue
    {
      public:
        typedef std::map<uint64_t, std::string> container_t;
        typedef std::set<std::string> keysContainer_t;

      public:
        int putValue(const std::string& _key, const std::string& _value);
        int getValue(const keysContainer_t& _keysToWait,
                     std::string* _updatedKey,
                     container_t* _values,
                     const std::chrono::system_clock::duration& _timeout);
    };
}

#endif /* KEYVALUE_H_ */
