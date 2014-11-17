// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "KeyValue.h"
#include "KeyValueGuard.h"

using namespace dds;

void CKeyValue::putValue(const std::string& _key, const std::string& _value)
{
    CKeyValueGuard::instance().putValue(_key, _value);
}