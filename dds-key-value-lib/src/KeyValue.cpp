// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "KeyValue.h"
#include "KeyValueGuard.h"

using namespace dds;
using namespace std;
using namespace MiscCommon;

void CKeyValue::putValue(const string& _key, const string& _value)
{
    SUpdateKeyCmd cmd;
    cmd.m_sKey = _key;
    cmd.m_sValue = _value;

    SCommandContainer cmd_container;
    cmd_container.m_cmdType = cmdUPDATE_KEY;
    cmd_container.m_cmd = cmd;

    CKeyValueGuard::instance().notifyAgent(&cmd_container);
}

// Return 0 when successful
// Return 1 when timed out earlier than update notification
int CKeyValue::getValue(const CKeyValue::keysContainer_t& _keysToWait,
                        std::string* _updatedKey,
                        container_t* _values,
                        const chrono::system_clock::duration& _timeout)
{
    if (_updatedKey == nullptr)
        throw invalid_argument("argument \"_updatedKey\" is NULL");

    SCommandContainer cmd_container;
    cmd_container.m_cmdType = cmdWAIT_FOR_KEY_UPDATE;
    cmd_container.m_sKeysToWait = _keysToWait;

    /*    mutex mutexKeyWait;
        unique_lock<mutex> lk(mutexKeyWait);
        auto now = std::chrono::system_clock::now();
        if (cmd_container.m_cvKeyWait.wait_until(lk, now + _timeout) == cv_status::timeout)
        {
            return 1;
        }
    */
    CKeyValueGuard::instance().notifyAgent(&cmd_container);
    *_updatedKey = cmd_container.m_sUpdatedKey;

    return 0;
}
