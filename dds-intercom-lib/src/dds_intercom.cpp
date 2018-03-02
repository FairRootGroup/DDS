// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "dds_intercom.h"
#include "DDSIntercomGuard.h"

using namespace dds;
using namespace dds::intercom_api;
using namespace dds::internal_api;
using namespace dds::protocol_api;
using namespace MiscCommon;
using namespace std;

CIntercomService::~CIntercomService()
{
    CDDSIntercomGuard::instance().stop();
}

void CIntercomService::subscribeOnError(errorSignal_t::slot_function_type _subscriber)
{
    connection_t connection = CDDSIntercomGuard::instance().connectError(_subscriber);
}

void CIntercomService::start(const std::string& _sessionID)
{
    CDDSIntercomGuard::instance().start(_sessionID);
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

CKeyValue::CKeyValue(CIntercomService& _service)
    : m_service(_service)
{
}

CKeyValue::~CKeyValue()
{
    unsubscribe();
}

void CKeyValue::putValue(const string& _key, const string& _value)
{
    CDDSIntercomGuard::instance().putValue(_key, _value);
}

void CKeyValue::subscribe(signal_t::slot_function_type _subscriber)
{
    connection_t connection = CDDSIntercomGuard::instance().connectKeyValue(_subscriber);
    LOG(info) << "User process is waiting for property keys updates.";
}

void CKeyValue::subscribeOnDelete(deleteSignal_t::slot_function_type _subscriber)
{
    connection_t connection = CDDSIntercomGuard::instance().connectKeyValueDelete(_subscriber);
    LOG(info) << "User process is waiting for property keys deletes.";
}

void CKeyValue::unsubscribe()
{
    LOG(info) << "Unsubscribing from KeyValue events.";
    CDDSIntercomGuard::instance().disconnectKeyValue();
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

CCustomCmd::CCustomCmd(CIntercomService& _service)
    : m_service(_service)
{
}

CCustomCmd::~CCustomCmd()
{
    unsubscribe();
}

void CCustomCmd::send(const std::string& _command, const std::string& _condition)
{
    CDDSIntercomGuard::instance().sendCustomCmd(_command, _condition);
}

void CCustomCmd::subscribe(signal_t::slot_function_type _subscriber)
{
    connection_t connection = CDDSIntercomGuard::instance().connectCustomCmd(_subscriber);
    LOG(info) << "User process is waiting for custom commands.";
}

void CCustomCmd::subscribeOnReply(replySignal_t::slot_function_type _subscriber)
{
    connection_t connection = CDDSIntercomGuard::instance().connectCustomCmdReply(_subscriber);
    LOG(info) << "User process is waiting for replys from custom commands.";
}

void CCustomCmd::unsubscribe()
{
    LOG(info) << "Unsubscribing from CustomCmd events.";
    CDDSIntercomGuard::instance().disconnectCustomCmd();
}
