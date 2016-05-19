// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "SubmitCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SSubmitCmd::SSubmitCmd()
    : m_nNumberOfAgents(0)
{
}

size_t SSubmitCmd::size() const
{
    return dsize(m_sCfgFile) + dsize(m_sRMSType) + dsize(m_nNumberOfAgents);
}

bool SSubmitCmd::operator==(const SSubmitCmd& val) const
{
    return (m_sCfgFile == val.m_sCfgFile && m_sRMSType == val.m_sRMSType && m_sPath == val.m_sPath &&
            m_nNumberOfAgents == val.m_nNumberOfAgents);
}

void SSubmitCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_sRMSType).get(m_sCfgFile).get(m_sPath).get(m_nNumberOfAgents);
}

void SSubmitCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_sRMSType).put(m_sCfgFile).put(m_sPath).put(m_nNumberOfAgents);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SSubmitCmd& val)
{
    _stream << "RMS type: " << val.m_sRMSType << "; Config: " << val.m_sCfgFile << "; Path: " << val.m_sPath
            << "; Number of agents: " << val.m_nNumberOfAgents;

    return _stream;
}

bool dds::protocol_api::operator!=(const SSubmitCmd& lhs, const SSubmitCmd& rhs)
{
    return !(lhs == rhs);
}
