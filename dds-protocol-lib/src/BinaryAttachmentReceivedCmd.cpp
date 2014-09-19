// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "BinaryAttachmentReceivedCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

void SBinaryAttachmentReceivedCmd::normalizeToLocal() const
{
    m_srcCommand = inet::_normalizeRead16(m_srcCommand);
    m_receivedFileSize = inet::_normalizeRead32(m_receivedFileSize);
    m_downloadTime = inet::_normalizeRead32(m_downloadTime);
}

void SBinaryAttachmentReceivedCmd::normalizeToRemote() const
{
    m_srcCommand = inet::_normalizeWrite16(m_srcCommand);
    m_receivedFileSize = inet::_normalizeWrite32(m_receivedFileSize);
    m_downloadTime = inet::_normalizeWrite32(m_downloadTime);
}

void SBinaryAttachmentReceivedCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    size_t idx(0);

    m_srcCommand = _data[idx++];
    m_srcCommand += (_data[idx++] << 8);

    m_receivedFileSize = _data[idx++];
    m_receivedFileSize += (_data[idx++] << 8);
    m_receivedFileSize += (_data[idx++] << 16);
    m_receivedFileSize += (_data[idx++] << 24);

    m_downloadTime = _data[idx++];
    m_downloadTime += (_data[idx++] << 8);
    m_downloadTime += (_data[idx++] << 16);
    m_downloadTime += (_data[idx++] << 24);

    vector<string> v;
    MiscCommon::BYTEVector_t::const_iterator iter = _data.begin();
    advance(iter, idx);
    MiscCommon::BYTEVector_t::const_iterator iter_end = _data.end();
    for (; iter != iter_end;)
    {
        string tmp((string::value_type*)(&(*iter)));
        v.push_back(tmp);
        advance(iter, tmp.size() + 1);
    }

    // there are so far only 2 string fields in this msg container
    if (v.size() != 2)
        throw runtime_error("SBinaryAttachmentReceivedCmd: can't import data. Number of fields doesn't match.");
    m_requestedFileName.assign(v[0]);
    m_receivedFilePath.assign(v[1]);
}

void SBinaryAttachmentReceivedCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_srcCommand & 0xFF);
    _data->push_back(m_srcCommand >> 8);

    _data->push_back(m_receivedFileSize & 0xFF);
    _data->push_back((m_receivedFileSize >> 8) & 0xFF);
    _data->push_back((m_receivedFileSize >> 16) & 0xFF);
    _data->push_back((m_receivedFileSize >> 24) & 0xFF);

    _data->push_back(m_downloadTime & 0xFF);
    _data->push_back((m_downloadTime >> 8) & 0xFF);
    _data->push_back((m_downloadTime >> 16) & 0xFF);
    _data->push_back((m_downloadTime >> 24) & 0xFF);

    copy(m_requestedFileName.begin(), m_requestedFileName.end(), back_inserter(*_data));
    _data->push_back('\0');

    copy(m_receivedFilePath.begin(), m_receivedFilePath.end(), back_inserter(*_data));
    _data->push_back('\0');
}
