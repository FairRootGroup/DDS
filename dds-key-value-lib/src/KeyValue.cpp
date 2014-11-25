// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
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

    CKeyValueGuard::instance().notifyAgent(cmd_container);
}

void CKeyValue::getValue(const string& _key, container_t* _values, const chrono::system_clock::duration& _timeout)
{
}