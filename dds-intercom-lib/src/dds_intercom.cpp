// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "DDSIntercomGuard.h"
#include "dds_intercom.h"

using namespace dds;
using namespace dds::intercom_api;
using namespace dds::internal_api;
using namespace dds::protocol_api;
using namespace MiscCommon;
using namespace std;

void CIntercomBase::subscribeOnError(errorSignal_t::slot_function_type _subscriber)
{
    connection_t connection = CDDSIntercomGuard::instance().connectError(_subscriber);
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

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

    return CDDSIntercomGuard::instance().updateKey(cmd);
}

void CKeyValue::getValues(const std::string& _key, valuesMap_t* _values)
{
    CDDSIntercomGuard::instance().initLock();
    CDDSIntercomGuard::instance().getValues(_key, _values);
}

void CKeyValue::subscribe(signal_t::slot_function_type _subscriber)
{
    connection_t connection = CDDSIntercomGuard::instance().connectKeyValue(_subscriber);

    CDDSIntercomGuard::instance().initLock();
    CDDSIntercomGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting for property keys updates.";
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

    return CDDSIntercomGuard::instance().sendCustomCmd(cmd);
}

void CCustomCmd::subscribe(signal_t::slot_function_type _subscriber)
{
    connection_t connection = CDDSIntercomGuard::instance().connectCustomCmd(_subscriber);

    CDDSIntercomGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting for custom commands.";
}

void CCustomCmd::subscribeOnReply(replySignal_t::slot_function_type _subscriber)
{
    connection_t connection = CDDSIntercomGuard::instance().connectCustomCmdReply(_subscriber);

    CDDSIntercomGuard::instance().initAgentConnection();
    LOG(info) << "User process is waiting for replys from custom commands.";
}

void CCustomCmd::unsubscribe()
{
    LOG(info) << "Unsubscribing from CustomCmd events.";
    CDDSIntercomGuard::instance().disconnectCustomCmd();
}
