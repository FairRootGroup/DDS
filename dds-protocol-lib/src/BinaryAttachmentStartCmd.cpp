// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "BinaryAttachmentStartCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

void SBinaryAttachmentStartCmd::normalizeToLocal() const
{
    m_fileCrc32 = inet::_normalizeRead32(m_fileCrc32);
    m_fileSize = inet::_normalizeRead32(m_fileSize);
    m_srcCommand = inet::_normalizeRead16(m_srcCommand);
}

void SBinaryAttachmentStartCmd::normalizeToRemote() const
{
    m_fileCrc32 = inet::_normalizeWrite32(m_fileCrc32);
    m_fileSize = inet::_normalizeWrite32(m_fileSize);
    m_srcCommand = inet::_normalizeWrite16(m_srcCommand);
}

void SBinaryAttachmentStartCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
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
}

void SBinaryAttachmentStartCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
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
}
