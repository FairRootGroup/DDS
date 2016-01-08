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
    : m_nRMSTypeCode(0)
{
}

size_t SSubmitCmd::size() const
{
    return dsize(m_sCfgFile) + dsize(m_nRMSTypeCode);
}

bool SSubmitCmd::operator==(const SSubmitCmd& val) const
{
    return (m_sCfgFile == val.m_sCfgFile && m_nRMSTypeCode == val.m_nRMSTypeCode);
}

void SSubmitCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_nRMSTypeCode).get(m_sCfgFile);
}

void SSubmitCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_nRMSTypeCode).put(m_sCfgFile);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SSubmitCmd& val)
{
    _stream << "RMS type code: " << val.m_nRMSTypeCode;
    if (val.m_nRMSTypeCode == SSubmitCmd::SSH)
        _stream << "; SSH Hosts config: " << val.m_sCfgFile;

    return _stream;
}

bool dds::protocol_api::operator!=(const SSubmitCmd& lhs, const SSubmitCmd& rhs)
{
    return !(lhs == rhs);
}
