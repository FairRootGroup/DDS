// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "UpdateKeyErrorCmd.h"
#include "INet.h"
#include <stdexcept>

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SUpdateKeyErrorCmd::SUpdateKeyErrorCmd()
    : m_serverCmd()
    , m_userCmd()
    , m_errorCode(0)
{
}

size_t SUpdateKeyErrorCmd::size() const
{
    return m_serverCmd.size() + m_userCmd.size() + dsize(m_errorCode);
}

bool SUpdateKeyErrorCmd::operator==(const SUpdateKeyErrorCmd& val) const
{
    return (m_serverCmd == val.m_serverCmd && m_userCmd == val.m_userCmd && m_errorCode == val.m_errorCode);
}

void SUpdateKeyErrorCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data)
        .get(m_serverCmd.m_sKey)
        .get(m_serverCmd.m_sValue)
        .get(m_serverCmd.m_version)
        .get(m_userCmd.m_sKey)
        .get(m_userCmd.m_sValue)
        .get(m_userCmd.m_version)
        .get(m_errorCode);
}

void SUpdateKeyErrorCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data)
        .put(m_serverCmd.m_sKey)
        .put(m_serverCmd.m_sValue)
        .put(m_serverCmd.m_version)
        .put(m_userCmd.m_sKey)
        .put(m_userCmd.m_sValue)
        .put(m_userCmd.m_version)
        .put(m_errorCode);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SUpdateKeyErrorCmd& val)
{
    return _stream << "Server: " << val.m_serverCmd << "; User: " << val.m_userCmd
                   << "; Error code: " << val.m_errorCode;
}

bool dds::protocol_api::operator!=(const SUpdateKeyErrorCmd& lhs, const SUpdateKeyErrorCmd& rhs)
{
    return !(lhs == rhs);
}
