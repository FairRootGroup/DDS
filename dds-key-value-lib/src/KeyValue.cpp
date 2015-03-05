// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "KeyValue.h"
#include "KeyValueGuard.h"

using namespace dds;
using namespace std;
using namespace MiscCommon;

// const std::chrono::system_clock::duration g_maxWaitTime = std::chrono::milliseconds(20000);

CKeyValue::~CKeyValue()
{
    unsubscribe();
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

    // auto now = std::chrono::system_clock::now();
    // unique_lock<mutex> lk(CKeyValueGuard::instance().m_syncHelper.m_mtxUpdateKey);
    // if (CKeyValueGuard::instance().m_syncHelper.m_cvUpdateKey.wait_until(lk, now + g_maxWaitTime) ==
    // cv_status::timeout)
    //{
    //    LOG(debug) << "CKeyValue::putValue timed out while updating key = " << _key << " with value = " << _value;
    //    return 1;
    //}

    return 0;
}

void CKeyValue::getValues(const std::string& _key, valuesMap_t* _values)
{
    CKeyValueGuard::instance().initLock();
    CKeyValueGuard::instance().getValues(_key, _values);
}

CKeyValue::connection_t CKeyValue::subscribe(signal_t::slot_function_type _subscriber)
{
    CKeyValueGuard::instance().initLock();
    CKeyValueGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting for property keys updates.";

    unsubscribe();
    m_signalConnection = CKeyValueGuard::instance().connect(_subscriber);

    return m_signalConnection;
}

void CKeyValue::unsubscribe()
{
    if (!m_signalConnection.connected())
        return;

    LOG(info) << "unsubscribing from key notification events.";
    m_signalConnection.disconnect();
}