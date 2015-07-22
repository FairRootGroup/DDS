// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "KeyValue.h"
#include "KeyValueGuard.h"

using namespace dds;
using namespace dds::key_value_api;
using namespace std;
using namespace MiscCommon;

CKeyValue::~CKeyValue()
{
    unsubscribe();
    unsubscribeError();
}

int CKeyValue::putValue(const string& _key, const string& _value)
{
    CKeyValueGuard::instance().initLock();
    CKeyValueGuard::instance().initAgentConnection();

    SUpdateKeyCmd cmd;
    cmd.m_sKey = _key;
    cmd.m_sValue = _value;
    if (1 == CKeyValueGuard::instance().updateKey(cmd))
        return 1;

    return 0;
}

void CKeyValue::getValues(const std::string& _key, valuesMap_t* _values)
{
    CKeyValueGuard::instance().initLock();
    CKeyValueGuard::instance().getValues(_key, _values);
}

void CKeyValue::subscribe(signal_t::slot_function_type _subscriber)
{
    CKeyValueGuard::instance().initLock();
    CKeyValueGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting for property keys updates.";

    connection_t connection = CKeyValueGuard::instance().connect(_subscriber);
}

void CKeyValue::unsubscribe()
{
    LOG(info) << "unsubscribing from key notification events.";
    CKeyValueGuard::instance().disconnect();
}

void CKeyValue::subscribeError(errorSignal_t::slot_function_type _subscriber)
{
    CKeyValueGuard::instance().initLock();
    CKeyValueGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting error messages.";

    connection_t connection = CKeyValueGuard::instance().connectError(_subscriber);
}

void CKeyValue::unsubscribeError()
{
    LOG(info) << "unsubscribing from error message notifications";
    CKeyValueGuard::instance().disconnectError();
}