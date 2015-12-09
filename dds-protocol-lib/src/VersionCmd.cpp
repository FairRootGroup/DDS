// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "VersionCmd.h"
// DDS
#include "ProtocolCommands.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SVersionCmd::SVersionCmd()
    : m_version(g_protocolCommandsVersion)
    , m_channelType(0)
{
}

size_t SVersionCmd::size() const
{
    return dsize(m_version) + dsize(m_channelType);
}

bool SVersionCmd::operator==(const SVersionCmd& val) const
{
    return (m_version == val.m_version) && (m_channelType == val.m_channelType);
}

void SVersionCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_version).get(m_channelType);
}

void SVersionCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_version).put(m_channelType);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SVersionCmd& val)
{
    return _stream << val.m_version << " " << val.m_channelType;
}

bool dds::protocol_api::operator!=(const SVersionCmd& lhs, const SVersionCmd& rhs)
{
    return !(lhs == rhs);
}