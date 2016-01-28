// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "UpdateKeyCmd.h"
#include "INet.h"
#include <stdexcept>

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SUpdateKeyCmd::SUpdateKeyCmd()
    : m_sKey()
    , m_sValue()
{
}

size_t SUpdateKeyCmd::size() const
{
    return dsize(m_sKey) + dsize(m_sValue);
}

bool SUpdateKeyCmd::operator==(const SUpdateKeyCmd& val) const
{
    return (m_sKey == val.m_sKey && m_sValue == val.m_sValue);
}

void SUpdateKeyCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_sKey).get(m_sValue);
}

void SUpdateKeyCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_sKey).put(m_sValue);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SUpdateKeyCmd& val)
{
    return _stream << "key: " << val.m_sKey << " Value: " << val.m_sValue;
}

bool dds::protocol_api::operator!=(const SUpdateKeyCmd& lhs, const SUpdateKeyCmd& rhs)
{
    return !(lhs == rhs);
}
