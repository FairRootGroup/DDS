// Copyright 2014 GSI, Inc. All rights reserved.
//
//
// DDS
#include "dds_intercom.h"
#include "IntercomServiceCore.h"

using namespace dds;
using namespace dds::intercom_api;
using namespace dds::internal_api;
using namespace dds::protocol_api;
using namespace MiscCommon;
using namespace std;

CIntercomService::~CIntercomService()
{
    CIntercomServiceCore::instance().stop();
}

void CIntercomService::subscribeOnError(errorSignal_t::slot_function_type _subscriber)
{
    connection_t connection = CIntercomServiceCore::instance().connectError(_subscriber);
}

void CIntercomService::subscribeOnTaskDone(taskDoneSignal_t::slot_function_type _subscriber)
{
    connection_t connection = CIntercomServiceCore::instance().connectKeyValueDelete(_subscriber);
    LOG(info) << "User process is waiting for property keys deletes.";
}

void CIntercomService::start(const std::string& _sessionID)
{
    CIntercomServiceCore::instance().start(_sessionID);
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
    CIntercomServiceCore::instance().putValue(_key, _value);
}

void CKeyValue::subscribe(signal_t::slot_function_type _subscriber)
{
    connection_t connection = CIntercomServiceCore::instance().connectKeyValue(_subscriber);
    LOG(info) << "User process is waiting for property keys updates.";
}

void CKeyValue::unsubscribe()
{
    LOG(info) << "Unsubscribing from KeyValue events.";
    CIntercomServiceCore::instance().disconnectKeyValue();
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
    CIntercomServiceCore::instance().sendCustomCmd(_command, _condition);
}

void CCustomCmd::subscribe(signal_t::slot_function_type _subscriber)
{
    connection_t connection = CIntercomServiceCore::instance().connectCustomCmd(_subscriber);
    LOG(info) << "User process is waiting for custom commands.";
}

void CCustomCmd::subscribeOnReply(replySignal_t::slot_function_type _subscriber)
{
    connection_t connection = CIntercomServiceCore::instance().connectCustomCmdReply(_subscriber);
    LOG(info) << "User process is waiting for replys from custom commands.";
}

void CCustomCmd::unsubscribe()
{
    LOG(info) << "Unsubscribing from CustomCmd events.";
    CIntercomServiceCore::instance().disconnectCustomCmd();
}
