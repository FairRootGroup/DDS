// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "BinaryAttachmentStartCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

void SBinaryAttachmentStartCmd::normalizeToLocal() const
{
    m_fileCrc32 = inet::normalizeRead(m_fileCrc32);
    m_fileSize = inet::normalizeRead(m_fileSize);
    m_srcCommand = inet::normalizeRead(m_srcCommand);
}

void SBinaryAttachmentStartCmd::normalizeToRemote() const
{
    m_fileCrc32 = inet::normalizeWrite(m_fileCrc32);
    m_fileSize = inet::normalizeWrite(m_fileSize);
    m_srcCommand = inet::normalizeWrite(m_srcCommand);
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

    inet::readData(&m_fileSize, &_data, &idx);
    inet::readData(&m_fileCrc32, &_data, &idx);
    inet::readData(&m_srcCommand, &_data, &idx);
}

void SBinaryAttachmentStartCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_fileName.begin(), m_fileName.end(), back_inserter(*_data));
    _data->push_back('\0');

    copy(m_fileId.begin(), m_fileId.end(), back_inserter(*_data));

    inet::pushData(m_fileSize, _data);
    inet::pushData(m_fileCrc32, _data);
    inet::pushData(m_srcCommand, _data);
}
