// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "BinaryAttachmentCmd.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;

SBinaryAttachmentCmd::SBinaryAttachmentCmd()
    : m_fileId()
    , m_offset(0)
    , m_size(0)
    , m_crc32(0)
    , m_data()
{
}

size_t SBinaryAttachmentCmd::size() const
{
    return dsize(m_fileId) + dsize(m_data) + dsize(m_offset) + dsize(m_size) + dsize(m_crc32);
}

bool SBinaryAttachmentCmd::operator==(const SBinaryAttachmentCmd& _val) const
{
    unsigned int i = 0;
    for (auto c : _val.m_data)
    {
        if (m_data[i++] != c)
            return false;
    }
    return (m_fileId == _val.m_fileId && m_offset == _val.m_offset && m_size == _val.m_size && m_crc32 == _val.m_crc32);
}

void SBinaryAttachmentCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_fileId).get(m_offset).get(m_size).get(m_crc32).get(m_data);
}

void SBinaryAttachmentCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_fileId).put(m_offset).put(m_size).put(m_crc32).put(m_data);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SBinaryAttachmentCmd& _val)
{
    _stream << "fileId=" << _val.m_fileId << " offset=" << _val.m_offset << " size=" << _val.m_size
            << " crc32=" << _val.m_crc32;
    for (const auto& c : _val.m_data)
    {
        _stream << c;
    }
    return _stream;
}

bool dds::protocol_api::operator!=(const SBinaryAttachmentCmd& lhs, const SBinaryAttachmentCmd& rhs)
{
    return !(lhs == rhs);
}
