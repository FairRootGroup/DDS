// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "DeleteKeyCmd.h"
#include <stdexcept>
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SDeleteKeyCmd::SDeleteKeyCmd()
    : m_sKey()
{
}

size_t SDeleteKeyCmd::size() const
{
    return dsize(m_sKey);
}

bool SDeleteKeyCmd::operator==(const SDeleteKeyCmd& val) const
{
    return (m_sKey == val.m_sKey);
}

void SDeleteKeyCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_sKey);
}

void SDeleteKeyCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_sKey);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SDeleteKeyCmd& val)
{
    return _stream << "key: " << val.m_sKey;
}

bool dds::protocol_api::operator!=(const SDeleteKeyCmd& lhs, const SDeleteKeyCmd& rhs)
{
    return !(lhs == rhs);
}
