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
    m_offset = inet::_normalizeRead32(m_offset);
    m_size = inet::_normalizeRead32(m_size);
    m_crc32 = inet::_normalizeRead32(m_crc32);
}

void SBinaryAttachmentCmd::normalizeToRemote() const
{
    m_offset = inet::_normalizeWrite32(m_offset);
    m_size = inet::_normalizeWrite32(m_size);
    m_crc32 = inet::_normalizeWrite32(m_crc32);
}

void SBinaryAttachmentCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    size_t idx(0);

    auto iter_id_begin = _data.begin() + idx;
    auto iter_id_end = iter_id_begin + m_fileId.size();
    copy(iter_id_begin, iter_id_end, m_fileId.begin());
    idx += m_fileId.size();

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
    copy(m_fileId.begin(), m_fileId.end(), back_inserter(*_data));

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
