// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "KeyValue.h"
#include "KeyValueGuard.h"

using namespace dds;
using namespace std;
using namespace MiscCommon;

const std::chrono::system_clock::duration g_maxWaitTime = std::chrono::seconds(60);

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
        return 1;
    }

    return 0;
}

void CKeyValue::getValues(const std::string& _key, valuesMap_t* _values)
{
    CKeyValueGuard::instance().getValues(_key, _values);
}

int CKeyValue::waitForUpdate(const std::chrono::system_clock::duration& _timeout)
{
    LOG(debug) << "User process is waiting for property keys updates.";
    CKeyValueGuard::instance().initAgentConnection();

    auto now = std::chrono::system_clock::now();
    unique_lock<mutex> lk(CKeyValueGuard::instance().m_syncHelper.m_mtxWaitKey);
    if (CKeyValueGuard::instance().m_syncHelper.m_cvWaitKey.wait_until(lk, now + _timeout) == cv_status::timeout)
    {
        LOG(debug) << "waiting for property keys updates has timed out";
        return 1;
    }
    LOG(debug) << "waiting for property keys updates is ended";

    return 0;
}
