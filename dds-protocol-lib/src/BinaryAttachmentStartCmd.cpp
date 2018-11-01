// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "BinaryAttachmentStartCmd.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;

SBinaryAttachmentStartCmd::SBinaryAttachmentStartCmd()
    : m_fileId()
    , m_fileName()
    , m_fileSize(0)
    , m_fileCrc32(0)
    , m_srcCommand(0)
{
}
size_t SBinaryAttachmentStartCmd::size() const
{
    return dsize(m_fileId) + dsize(m_fileName) + dsize(m_fileSize) + dsize(m_fileCrc32) + dsize(m_srcCommand);
}

bool SBinaryAttachmentStartCmd::operator==(const SBinaryAttachmentStartCmd& _val) const
{
    return (m_fileId == _val.m_fileId && m_fileCrc32 == _val.m_fileCrc32 && m_fileName == _val.m_fileName &&
            m_fileSize == _val.m_fileSize && m_srcCommand == _val.m_srcCommand);
}

void SBinaryAttachmentStartCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_fileId).get(m_fileName).get(m_fileSize).get(m_fileCrc32).get(m_srcCommand);
}

void SBinaryAttachmentStartCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_fileId).put(m_fileName).put(m_fileSize).put(m_fileCrc32).put(m_srcCommand);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SBinaryAttachmentStartCmd& _val)
{
    _stream << "fileId=" << _val.m_fileId << " fileName=" << _val.m_fileName << " fileSize=" << _val.m_fileSize
            << " fileCrc32=" << _val.m_fileCrc32;
    return _stream;
}

bool dds::protocol_api::operator!=(const SBinaryAttachmentStartCmd& lhs, const SBinaryAttachmentStartCmd& rhs)
{
    return !(lhs == rhs);
}
