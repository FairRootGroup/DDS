// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "BinaryDownloadStatCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

void SBinaryDownloadStatCmd::normalizeToLocal() const
{
    m_recievedFileSize = inet::_normalizeRead32(m_recievedFileSize);
    m_recievedCrc32 = inet::_normalizeRead32(m_recievedCrc32);
    m_downloadTime = inet::_normalizeRead32(m_downloadTime);
    m_srcCommand = inet::_normalizeRead16(m_srcCommand);
}

void SBinaryDownloadStatCmd::normalizeToRemote() const
{
    m_recievedFileSize = inet::_normalizeWrite32(m_recievedFileSize);
    m_recievedCrc32 = inet::_normalizeWrite32(m_recievedCrc32);
    m_downloadTime = inet::_normalizeWrite32(m_downloadTime);
    m_srcCommand = inet::_normalizeWrite16(m_srcCommand);
}

void SBinaryDownloadStatCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    size_t idx(0);
    m_recievedFileSize = _data[idx++];
    m_recievedFileSize += (_data[idx++] << 8);
    m_recievedFileSize += (_data[idx++] << 16);
    m_recievedFileSize += (_data[idx++] << 24);

    m_recievedCrc32 = _data[idx++];
    m_recievedCrc32 += (_data[idx++] << 8);
    m_recievedCrc32 += (_data[idx++] << 16);
    m_recievedCrc32 += (_data[idx++] << 24);

    m_downloadTime = _data[idx++];
    m_downloadTime += (_data[idx++] << 8);
    m_downloadTime += (_data[idx++] << 16);
    m_downloadTime += (_data[idx++] << 24);

    m_srcCommand = _data[idx++];
    m_srcCommand += (_data[idx++] << 8);
}

void SBinaryDownloadStatCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_recievedFileSize & 0xFF);
    _data->push_back((m_recievedFileSize >> 8) & 0xFF);
    _data->push_back((m_recievedFileSize >> 16) & 0xFF);
    _data->push_back((m_recievedFileSize >> 24) & 0xFF);

    _data->push_back(m_recievedCrc32 & 0xFF);
    _data->push_back((m_recievedCrc32 >> 8) & 0xFF);
    _data->push_back((m_recievedCrc32 >> 16) & 0xFF);
    _data->push_back((m_recievedCrc32 >> 24) & 0xFF);

    _data->push_back(m_downloadTime & 0xFF);
    _data->push_back((m_downloadTime >> 8) & 0xFF);
    _data->push_back((m_downloadTime >> 16) & 0xFF);
    _data->push_back((m_downloadTime >> 24) & 0xFF);

    _data->push_back(m_srcCommand & 0xFF);
    _data->push_back(m_srcCommand >> 8);
}
