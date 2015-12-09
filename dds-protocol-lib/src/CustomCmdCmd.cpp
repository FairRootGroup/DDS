// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "CustomCmdCmd.h"
#include <stdexcept>
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SCustomCmdCmd::SCustomCmdCmd()
    : m_senderId(0)
    , m_sCmd()
    , m_sCondition()
{
}

size_t SCustomCmdCmd::size() const
{
    return dsize(m_senderId) + dsize(m_sCmd) + dsize(m_sCondition);
}

bool SCustomCmdCmd::operator==(const SCustomCmdCmd& val) const
{
    return (m_senderId == val.m_senderId && m_sCmd == val.m_sCmd && m_sCondition == val.m_sCondition);
}

void SCustomCmdCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_senderId).get(m_sCmd).get(m_sCondition);
}

void SCustomCmdCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_senderId).put(m_sCmd).put(m_sCondition);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SCustomCmdCmd& val)
{
    return _stream << "senderId:" << val.m_senderId << "cmd: " << val.m_sCmd << " condition: " << val.m_sCondition;
}

bool dds::protocol_api::operator!=(const SCustomCmdCmd& lhs, const SCustomCmdCmd& rhs)
{
    return !(lhs == rhs);
}
