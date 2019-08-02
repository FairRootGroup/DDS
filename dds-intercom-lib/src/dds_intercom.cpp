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

CIntercomService::CIntercomService()
    : m_impl(make_shared<CIntercomServiceCore>())
{
}

CIntercomService::~CIntercomService()
{
    m_impl->stop();
}

void CIntercomService::subscribeOnError(errorSignal_t::slot_function_type _subscriber)
{
    connection_t connection = m_impl->connectError(_subscriber);
    LOG(info) << "User process is waiting error events.";
}

void CIntercomService::subscribeOnTaskDone(taskDoneSignal_t::slot_function_type _subscriber)
{
    connection_t connection = m_impl->connectKeyValueTaskDone(_subscriber);
    LOG(info) << "User process is waiting for task done events.";
}

void CIntercomService::start(const std::string& _sessionID)
{
    m_impl->start(_sessionID);
}

void CIntercomService::waitCondition()
{
    m_impl->waitCondition();
}

void CIntercomService::stopCondition()
{
    m_impl->stopCondition();
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
    m_service.m_impl->putValue(_key, _value);
}

void CKeyValue::subscribe(signal_t::slot_function_type _subscriber)
{
    connection_t connection = m_service.m_impl->connectKeyValue(_subscriber);
    LOG(info) << "User process is waiting for property keys updates.";
}

void CKeyValue::unsubscribe()
{
    LOG(info) << "Unsubscribing from KeyValue events.";
    m_service.m_impl->disconnectKeyValue();
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
    m_service.m_impl->sendCustomCmd(_command, _condition);
}

void CCustomCmd::subscribe(signal_t::slot_function_type _subscriber)
{
    connection_t connection = m_service.m_impl->connectCustomCmd(_subscriber);
    LOG(info) << "User process is waiting for custom commands.";
}

void CCustomCmd::subscribeOnReply(replySignal_t::slot_function_type _subscriber)
{
    connection_t connection = m_service.m_impl->connectCustomCmdReply(_subscriber);
    LOG(info) << "User process is waiting for replys from custom commands.";
}

void CCustomCmd::unsubscribe()
{
    LOG(info) << "Unsubscribing from CustomCmd events.";
    m_service.m_impl->disconnectCustomCmd();
}
