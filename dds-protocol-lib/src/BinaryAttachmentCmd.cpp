// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "BinaryAttachmentCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

void SBinaryAttachmentCmd::normalizeToLocal()
{
    m_crc32 = inet::_normalizeRead32(m_crc32);
    m_fileSize = inet::_normalizeRead32(m_fileSize);
}

void SBinaryAttachmentCmd::normalizeToRemote()
{
    m_crc32 = inet::_normalizeWrite32(m_crc32);
    m_fileSize = inet::_normalizeWrite32(m_fileSize);
}

void SBinaryAttachmentCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    size_t idx(0);
    MiscCommon::BYTEVector_t::const_iterator iter = _data.begin();
    MiscCommon::BYTEVector_t::const_iterator iter_end = _data.end();

    for (; iter != iter_end; ++iter, ++idx)
    {
        char c(*iter);
        if ('\0' == c)
        {
            ++iter;
            ++idx;
            break;
        }
        m_fileName.push_back(c);
    }

    m_fileSize = _data[idx++];
    m_fileSize += (_data[idx++] << 8);
    m_fileSize += (_data[idx++] << 16);
    m_fileSize += (_data[idx++] << 24);

    m_crc32 = _data[idx++];
    m_crc32 += (_data[idx++] << 8);
    m_crc32 += (_data[idx++] << 16);
    m_crc32 += (_data[idx++] << 24);

    m_fileData.assign(_data.begin() + idx, _data.end());
}

void SBinaryAttachmentCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_fileName.begin(), m_fileName.end(), back_inserter(*_data));
    _data->push_back('\0');

    _data->push_back(m_fileSize & 0xFF);
    _data->push_back((m_fileSize >> 8) & 0xFF);
    _data->push_back((m_fileSize >> 16) & 0xFF);
    _data->push_back((m_fileSize >> 24) & 0xFF);

    _data->push_back(m_crc32 & 0xFF);
    _data->push_back((m_crc32 >> 8) & 0xFF);
    _data->push_back((m_crc32 >> 16) & 0xFF);
    _data->push_back((m_crc32 >> 24) & 0xFF);

    copy(m_fileData.begin(), m_fileData.end(), back_inserter(*_data));
}
