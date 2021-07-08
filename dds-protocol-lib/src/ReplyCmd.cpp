// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "ReplyCmd.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
using namespace dds::misc;

SReplyCmd::SReplyCmd()
    : m_statusCode(0)
    , m_returnCode(0)
    , m_srcCommand(0)
    , m_sMsg()
{
}

SReplyCmd::SReplyCmd(const std::string& _msg, uint16_t _statusCode, uint16_t _returnCode, uint16_t _srcCommand)
    : m_statusCode(_statusCode)
    , m_returnCode(_returnCode)
    , m_srcCommand(_srcCommand)
    , m_sMsg(_msg)
{
}

size_t SReplyCmd::size() const
{
    return dsize(m_sMsg) + dsize(m_statusCode) + dsize(m_returnCode) + dsize(m_srcCommand);
}

bool SReplyCmd::operator==(const SReplyCmd& val) const
{
    return (m_sMsg == val.m_sMsg && m_statusCode == val.m_statusCode && m_returnCode == val.m_returnCode &&
            m_srcCommand == val.m_srcCommand);
}

void SReplyCmd::_convertFromData(const BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_statusCode).get(m_returnCode).get(m_srcCommand).get(m_sMsg);
}

void SReplyCmd::_convertToData(BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_statusCode).put(m_returnCode).put(m_srcCommand).put(m_sMsg);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SReplyCmd& val)
{
    return _stream << "Status code: " << val.m_statusCode << "; return code: " << val.m_returnCode
                   << "; source command: " << val.m_srcCommand << "; message: " << val.m_sMsg;
}

bool dds::protocol_api::operator!=(const SReplyCmd& lhs, const SReplyCmd& rhs)
{
    return !(lhs == rhs);
}
