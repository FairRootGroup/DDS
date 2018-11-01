// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "UUIDCmd.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;

SIDCmd::SIDCmd()
    : m_id()
{
}

size_t SIDCmd::size() const
{
    return dsize(m_id);
}

bool SIDCmd::operator==(const SIDCmd& _val) const
{
    return (m_id == _val.m_id);
}

void SIDCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_id);
}

void SIDCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_id);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SIDCmd& _val)
{
    _stream << _val.m_id;
    return _stream;
}

bool dds::protocol_api::operator!=(const SIDCmd& lhs, const SIDCmd& rhs)
{
    return !(lhs == rhs);
}
