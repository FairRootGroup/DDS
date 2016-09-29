// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "HostInfoCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SHostInfoCmd::SHostInfoCmd()
    : m_agentPid(0)
    , m_submitTime(0)
    , m_username()
    , m_host()
    , m_version()
    , m_DDSPath()
    , m_workerId()
{
}

size_t SHostInfoCmd::size() const
{
    return dsize(m_username) + dsize(m_host) + dsize(m_agentPid) + dsize(m_submitTime) + dsize(m_version) +
           dsize(m_DDSPath) + dsize(m_workerId);
}

bool SHostInfoCmd::operator==(const SHostInfoCmd& val) const
{
    return (m_username == val.m_username && m_host == val.m_host && m_version == val.m_version &&
            m_DDSPath == val.m_DDSPath && m_agentPid == val.m_agentPid && m_submitTime == val.m_submitTime &&
            m_workerId == val.m_workerId);
}

void SHostInfoCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data)
        .get(m_agentPid)
        .get(m_submitTime)
        .get(m_username)
        .get(m_host)
        .get(m_version)
        .get(m_DDSPath)
        .get(m_workerId);
}

void SHostInfoCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data)
        .put(m_agentPid)
        .put(m_submitTime)
        .put(m_username)
        .put(m_host)
        .put(m_version)
        .put(m_DDSPath)
        .put(m_workerId);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SHostInfoCmd& val)
{
    _stream << val.m_username << ":" << val.m_host << ": " << val.m_version << ":" << val.m_DDSPath << "; agent ["
            << val.m_agentPid << "]; startup time: " << val.m_submitTime << "; worker ID:" << val.m_workerId;
    return _stream;
}

bool dds::protocol_api::operator!=(const SHostInfoCmd& lhs, const SHostInfoCmd& rhs)
{
    return !(lhs == rhs);
}
