// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "SimpleMsgCmd.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;

SSimpleMsgCmd::SSimpleMsgCmd()
    : m_msgSeverity(0)
    , m_srcCommand(0)
    , m_sMsg()
{
}

SSimpleMsgCmd::SSimpleMsgCmd(const std::string& _msg, uint16_t _severity, uint16_t _command)
    : m_msgSeverity(_severity)
    , m_srcCommand(_command)
    , m_sMsg(_msg)
{
}

size_t SSimpleMsgCmd::size() const
{
    return dsize(m_sMsg) + dsize(m_msgSeverity) + dsize(m_srcCommand);
}

bool SSimpleMsgCmd::operator==(const SSimpleMsgCmd& val) const
{
    return (m_sMsg == val.m_sMsg && m_msgSeverity == val.m_msgSeverity && m_srcCommand == val.m_srcCommand);
}

void SSimpleMsgCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_msgSeverity).get(m_srcCommand).get(m_sMsg);
}

void SSimpleMsgCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_msgSeverity).put(m_srcCommand).put(m_sMsg);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SSimpleMsgCmd& val)
{
    return _stream << "source command: " << val.m_srcCommand << "; severity " << val.m_msgSeverity
                   << "; Msg: " << val.m_sMsg;
}

bool dds::protocol_api::operator!=(const SSimpleMsgCmd& lhs, const SSimpleMsgCmd& rhs)
{
    return !(lhs == rhs);
}
