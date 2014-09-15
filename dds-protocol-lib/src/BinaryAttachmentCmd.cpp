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

void SBinaryAttachmentCmd::normalizeToLocal() const
{
    m_fileCrc32 = inet::_normalizeRead32(m_fileCrc32);
    m_fileSize = inet::_normalizeRead32(m_fileSize);
    m_srcCommand = inet::_normalizeRead16(m_srcCommand);
    m_offset = inet::_normalizeRead32(m_offset);
    m_size = inet::_normalizeRead32(m_size);
    m_crc32 = inet::_normalizeRead32(m_crc32);
}

void SBinaryAttachmentCmd::normalizeToRemote() const
{
    m_fileCrc32 = inet::_normalizeWrite32(m_fileCrc32);
    m_fileSize = inet::_normalizeWrite32(m_fileSize);
    m_srcCommand = inet::_normalizeWrite16(m_srcCommand);
    m_offset = inet::_normalizeWrite32(m_offset);
    m_size = inet::_normalizeWrite32(m_size);
    m_crc32 = inet::_normalizeWrite32(m_crc32);
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

    auto iter_id_begin = _data.begin() + idx;
    auto iter_id_end = iter_id_begin + m_fileId.size();
    copy(iter_id_begin, iter_id_end, m_fileId.begin());
    idx += m_fileId.size();

    m_fileSize = _data[idx++];
    m_fileSize += (_data[idx++] << 8);
    m_fileSize += (_data[idx++] << 16);
    m_fileSize += (_data[idx++] << 24);

    m_fileCrc32 = _data[idx++];
    m_fileCrc32 += (_data[idx++] << 8);
    m_fileCrc32 += (_data[idx++] << 16);
    m_fileCrc32 += (_data[idx++] << 24);

    m_srcCommand = _data[idx++];
    m_srcCommand += (_data[idx++] << 8);

    m_offset = _data[idx++];
    m_offset += (_data[idx++] << 8);
    m_offset += (_data[idx++] << 16);
    m_offset += (_data[idx++] << 24);

    m_size = _data[idx++];
    m_size += (_data[idx++] << 8);
    m_size += (_data[idx++] << 16);
    m_size += (_data[idx++] << 24);

    m_crc32 = _data[idx++];
    m_crc32 += (_data[idx++] << 8);
    m_crc32 += (_data[idx++] << 16);
    m_crc32 += (_data[idx++] << 24);

    m_data.assign(_data.begin() + idx, _data.end());
}

void SBinaryAttachmentCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_fileName.begin(), m_fileName.end(), back_inserter(*_data));
    _data->push_back('\0');

    copy(m_fileId.begin(), m_fileId.end(), back_inserter(*_data));

    _data->push_back(m_fileSize & 0xFF);
    _data->push_back((m_fileSize >> 8) & 0xFF);
    _data->push_back((m_fileSize >> 16) & 0xFF);
    _data->push_back((m_fileSize >> 24) & 0xFF);

    _data->push_back(m_fileCrc32 & 0xFF);
    _data->push_back((m_fileCrc32 >> 8) & 0xFF);
    _data->push_back((m_fileCrc32 >> 16) & 0xFF);
    _data->push_back((m_fileCrc32 >> 24) & 0xFF);

    _data->push_back(m_srcCommand & 0xFF);
    _data->push_back(m_srcCommand >> 8);

    _data->push_back(m_offset & 0xFF);
    _data->push_back((m_offset >> 8) & 0xFF);
    _data->push_back((m_offset >> 16) & 0xFF);
    _data->push_back((m_offset >> 24) & 0xFF);

    _data->push_back(m_size & 0xFF);
    _data->push_back((m_size >> 8) & 0xFF);
    _data->push_back((m_size >> 16) & 0xFF);
    _data->push_back((m_size >> 24) & 0xFF);

    _data->push_back(m_crc32 & 0xFF);
    _data->push_back((m_crc32 >> 8) & 0xFF);
    _data->push_back((m_crc32 >> 16) & 0xFF);
    _data->push_back((m_crc32 >> 24) & 0xFF);

    copy(m_data.begin(), m_data.end(), back_inserter(*_data));
}
