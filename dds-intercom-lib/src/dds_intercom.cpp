// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "dds_intercom.h"
#include "DDSIntercomGuard.h"

using namespace dds;
using namespace dds::internal_api;
using namespace dds::protocol_api;
using namespace MiscCommon;
using namespace std;

CKeyValue::~CKeyValue()
{
    unsubscribe();
}

int CKeyValue::putValue(const string& _key, const string& _value)
{
    CDDSIntercomGuard::instance().initLock();
    CDDSIntercomGuard::instance().initAgentConnection();

    SUpdateKeyCmd cmd;
    cmd.m_sKey = _key;
    cmd.m_sValue = _value;
    if (1 == CDDSIntercomGuard::instance().updateKey(cmd))
        return 1;

    return 0;
}

void CKeyValue::getValues(const std::string& _key, valuesMap_t* _values)
{
    CDDSIntercomGuard::instance().initLock();
    CDDSIntercomGuard::instance().getValues(_key, _values);
}

void CKeyValue::subscribe(signal_t::slot_function_type _subscriber)
{
    CDDSIntercomGuard::instance().initLock();
    CDDSIntercomGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting for property keys updates.";

    connection_t connection = CDDSIntercomGuard::instance().connectKeyValue(_subscriber);
}

void CKeyValue::subscribeError(errorSignal_t::slot_function_type _subscriber)
{
    CDDSIntercomGuard::instance().initLock();
    CDDSIntercomGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting error messages.";

    connection_t connection = CDDSIntercomGuard::instance().connectKeyValueError(_subscriber);
}

void CKeyValue::unsubscribe()
{
    LOG(info) << "Unsubscribing from KeyValue events.";
    CDDSIntercomGuard::instance().disconnectKeyValue();
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

CCustomCmd::~CCustomCmd()
{
    unsubscribe();
}

int CCustomCmd::send(const std::string& _command, const std::string& _condition)
{
    CDDSIntercomGuard::instance().initAgentConnection();

    SCustomCmdCmd cmd;
    cmd.m_sCmd = _command;
    cmd.m_sCondition = _condition;
    if (1 == CDDSIntercomGuard::instance().sendCustomCmd(cmd))
        return 1;

    return 0;
}

void CCustomCmd::subscribe(signal_t::slot_function_type _subscriber)
{
    CDDSIntercomGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting for custom commands.";

    connection_t connection = CDDSIntercomGuard::instance().connectCustomCmd(_subscriber);
}

void CCustomCmd::subscribeReply(replySignal_t::slot_function_type _subscriber)
{
    CDDSIntercomGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting for replys from custom commands.";

    connection_t connection = CDDSIntercomGuard::instance().connectCustomCmdReply(_subscriber);
}

void CCustomCmd::unsubscribe()
{
    LOG(info) << "Unsubscribing from CustomCmd events.";
    CDDSIntercomGuard::instance().disconnectCustomCmd();
}
