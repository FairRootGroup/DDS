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
    class CKeyValue
    {
      public:
        typedef std::map<std::string, std::string> valuesMap_t;

      public:
        int putValue(const std::string& _key, const std::string& _value);
        void getValues(const std::string& _key, valuesMap_t* _values);
        int waitForUpdate(const std::chrono::system_clock::duration& _timeout);
        //        int getValue(const keysContainer_t& _keysToWait,
        //                     std::string* _updatedKey,
        //                     container_t* _values,
        //                     const std::chrono::system_clock::duration& _timeout);
    };
}

#endif /* KEYVALUE_H_ */
