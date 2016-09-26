// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Task.h"

using namespace std;
using namespace dds;
using namespace dds_octopus;
using namespace dds_octopus_task;

COctopusTask::COctopusTask()
    : m_customCmd(m_intercomService)
{
}

COctopusTask::~COctopusTask()
{
}

void COctopusTask::init()
{
    m_customCmd.subscribe(boost::bind(&COctopusTask::_onCustomCmd, this, _1, _2, _3));
    m_intercomService.start();

    std::unique_lock<std::mutex> lk(m_waitMutex);
    m_waitCondition.wait(lk);
}

void COctopusTask::onGetPingCmd(const SOctopusProtocol_GetPing& /*_cmd*/, uint64_t _senderId)
{
    SOctopusProtocol_Ping ping;
    boost::property_tree::ptree root;
    ping.get(&root);
    stringstream ss;
    boost::property_tree::write_json(ss, root);
    m_customCmd.send(ss.str(), to_string(_senderId));
}
