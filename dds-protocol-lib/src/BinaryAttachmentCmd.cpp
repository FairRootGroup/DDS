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
    m_offset = inet::normalizeRead(m_offset);
    m_size = inet::normalizeRead(m_size);
    m_crc32 = inet::normalizeRead(m_crc32);
}

void SBinaryAttachmentCmd::normalizeToRemote() const
{
    m_offset = inet::normalizeWrite(m_offset);
    m_size = inet::normalizeWrite(m_size);
    m_crc32 = inet::normalizeWrite(m_crc32);
}

void SBinaryAttachmentCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    size_t idx(0);

    auto iter_id_begin = _data.begin() + idx;
    auto iter_id_end = iter_id_begin + m_fileId.size();
    copy(iter_id_begin, iter_id_end, m_fileId.begin());
    idx += m_fileId.size();

    inet::readData(&m_offset, &_data, &idx);
    inet::readData(&m_size, &_data, &idx);
    inet::readData(&m_crc32, &_data, &idx);

    m_data.assign(_data.begin() + idx, _data.end());
}

void SBinaryAttachmentCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_fileId.begin(), m_fileId.end(), back_inserter(*_data));

    inet::pushData(m_offset, _data);
    inet::pushData(m_size, _data);
    inet::pushData(m_crc32, _data);

    copy(m_data.begin(), m_data.end(), back_inserter(*_data));
}
