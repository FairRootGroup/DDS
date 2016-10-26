// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Task.h"
// BOOST
#include <boost/crc.hpp>

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
    SOctopusProtocol_Return ret;
    boost::property_tree::ptree root;
    ret.get(&root);
    stringstream ss;
    boost::property_tree::write_json(ss, root);
    m_customCmd.send(ss.str(), to_string(_senderId));
}

void COctopusTask::onBigCmdCmd(const dds_octopus::SOctopusProtocol_BigCmd& _cmd, uint64_t _senderId)
{
    boost::crc_32_type bufCrc32;
    bufCrc32.process_bytes(_cmd.m_sVal.data(), _cmd.m_sVal.length());
    uint32_t cmdCrc32 = bufCrc32.checksum();

    SOctopusProtocol_Return ret;
    boost::property_tree::ptree root;
    ret.m_sVal = to_string(cmdCrc32);
    ret.get(&root);
    stringstream ss;
    boost::property_tree::write_json(ss, root);
    m_customCmd.send(ss.str(), to_string(_senderId));
}
