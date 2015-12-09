// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "BinaryAttachmentReceivedCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SBinaryAttachmentReceivedCmd::SBinaryAttachmentReceivedCmd()
    : m_receivedFilePath()
    , m_requestedFileName()
    , m_srcCommand(0)
    , m_receivedFileSize(0)
    , m_downloadTime(0)
{
}
size_t SBinaryAttachmentReceivedCmd::size() const
{
    return dsize(m_receivedFilePath) + dsize(m_requestedFileName) + dsize(m_srcCommand) + dsize(m_receivedFileSize) +
           dsize(m_downloadTime);
}

bool SBinaryAttachmentReceivedCmd::operator==(const SBinaryAttachmentReceivedCmd& _val) const
{
    return (m_receivedFilePath == _val.m_receivedFilePath && m_requestedFileName == _val.m_requestedFileName &&
            m_srcCommand == _val.m_srcCommand && m_receivedFileSize == _val.m_receivedFileSize &&
            m_downloadTime == _val.m_downloadTime);
}

void SBinaryAttachmentReceivedCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data)
        .get(m_srcCommand)
        .get(m_receivedFileSize)
        .get(m_downloadTime)
        .get(m_requestedFileName)
        .get(m_receivedFilePath);
}

void SBinaryAttachmentReceivedCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data)
        .put(m_srcCommand)
        .put(m_receivedFileSize)
        .put(m_downloadTime)
        .put(m_requestedFileName)
        .put(m_receivedFilePath);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SBinaryAttachmentReceivedCmd& _val)
{
    _stream << "receivedFilePath=" << _val.m_receivedFilePath << " requestedFileName=" << _val.m_requestedFileName
            << " receivedFileSize=" << _val.m_receivedFileSize << " downloadTime=" << _val.m_downloadTime;
    return _stream;
}

bool dds::protocol_api::operator!=(const SBinaryAttachmentReceivedCmd& lhs, const SBinaryAttachmentReceivedCmd& rhs)
{
    return !(lhs == rhs);
}
