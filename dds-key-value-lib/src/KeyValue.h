// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef KEYVALUE_H_
#define KEYVALUE_H_

#include <string>

namespace dds
{
    class CKeyValue
    {
      public:
        void putValue(const std::string& _key, const std::string& _value);
    };
}

#endif /* KEYVALUE_H_ */
