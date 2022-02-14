// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "HostInfoCmd.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
using namespace dds::misc;

SHostInfoCmd::SHostInfoCmd()
    : m_agentPid(0)
    , m_slots(0)
    , m_submitTime(0)
{
}

size_t SHostInfoCmd::size() const
{
    return dsize(m_username) + dsize(m_host) + dsize(m_agentPid) + dsize(m_slots) + dsize(m_submitTime) +
           dsize(m_version) + dsize(m_DDSPath) + dsize(m_workerId) + dsize(m_groupName);
}

bool SHostInfoCmd::operator==(const SHostInfoCmd& val) const
{
    return (m_username == val.m_username && m_host == val.m_host && m_version == val.m_version &&
            m_DDSPath == val.m_DDSPath && m_agentPid == val.m_agentPid && m_slots == val.m_slots &&
            m_submitTime == val.m_submitTime && m_submitTime == val.m_submitTime && m_workerId == val.m_workerId &&
            m_groupName == val.m_groupName);
}

void SHostInfoCmd::_convertFromData(const BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data)
        .get(m_agentPid)
        .get(m_slots)
        .get(m_submitTime)
        .get(m_username)
        .get(m_host)
        .get(m_version)
        .get(m_DDSPath)
        .get(m_workerId)
        .get(m_groupName);
}

void SHostInfoCmd::_convertToData(BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data)
        .put(m_agentPid)
        .put(m_slots)
        .put(m_submitTime)
        .put(m_username)
        .put(m_host)
        .put(m_version)
        .put(m_DDSPath)
        .put(m_workerId)
        .put(m_groupName);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SHostInfoCmd& val)
{
    _stream << "host: " << val.m_username << "@" << val.m_host << "; ver." << val.m_version
            << "; dds path:" << val.m_DDSPath << "; agent [" << val.m_agentPid << "]; task slots: " << val.m_slots
            << "; startup time: " << val.m_submitTime << "; worker ID: " << val.m_workerId
            << "; group Name: " << val.m_groupName;
    return _stream;
}

bool dds::protocol_api::operator!=(const SHostInfoCmd& lhs, const SHostInfoCmd& rhs)
{
    return !(lhs == rhs);
}
