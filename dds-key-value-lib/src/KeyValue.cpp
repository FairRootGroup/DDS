// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "KeyValue.h"
#include "KeyValueGuard.h"

using namespace dds;
using namespace std;
using namespace MiscCommon;

const std::chrono::system_clock::duration g_maxWaitTime = std::chrono::milliseconds(20000);

int CKeyValue::putValue(const string& _key, const string& _value)
{
    CKeyValueGuard::instance().initAgentConnection();

    SUpdateKeyCmd cmd;
    cmd.m_sKey = _key;
    cmd.m_sValue = _value;
    if (1 == CKeyValueGuard::instance().updateKey(cmd))
        return 1;

    auto now = std::chrono::system_clock::now();
    unique_lock<mutex> lk(CKeyValueGuard::instance().m_syncHelper.m_mtxUpdateKey);
    if (CKeyValueGuard::instance().m_syncHelper.m_cvUpdateKey.wait_until(lk, now + g_maxWaitTime) == cv_status::timeout)
    {
        LOG(debug) << "CKeyValue::putValue timed out while updating key = " << _key << " with value = " << _value;
        return 1;
    }

    return 0;
}

void CKeyValue::getValues(const std::string& _key, valuesMap_t* _values)
{
    CKeyValueGuard::instance().getValues(_key, _values);
}

CKeyValue::connection_t CKeyValue::subscribe(signal_t::slot_function_type _subscriber)
{
    LOG(debug) << "User process is waiting for property keys updates.";
    return CKeyValueGuard::instance().connect(_subscriber);
}
