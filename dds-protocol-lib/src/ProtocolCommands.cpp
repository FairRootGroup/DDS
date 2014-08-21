// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "ProtocolCommands.h"
// STD
#include <stdint.h>
#include <stdexcept>
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

//----------------------------------------------------------------------

void SVersionCmd::normalizeToLocal()
{
    m_version = inet::_normalizeRead16(m_version);
}

void SVersionCmd::normalizeToRemote()
{
    m_version = inet::_normalizeWrite16(m_version);
}

void SVersionCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "VersionCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    m_version = _data[0];
    m_version += (_data[1] << 8);
}

void SVersionCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_version & 0xFF);
    _data->push_back(m_version >> 8);
}

//----------------------------------------------------------------------

void SSubmitCmd::normalizeToLocal()
{
}

void SSubmitCmd::normalizeToRemote()
{
}

void SSubmitCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "SubmitCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    size_t idx(0);
    m_nRMSTypeCode = _data[idx++];
    m_nRMSTypeCode += (_data[idx] << 8);

    ++idx;
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
        throw runtime_error("SubmitCmd: can't import data. Number of fields doesn't match.");

    m_sTopoFile.assign(v[0]);
    m_sSSHCfgFile.assign(v[1]);
}

void SSubmitCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_nRMSTypeCode & 0xFF);
    _data->push_back(m_nRMSTypeCode >> 8);

    copy(m_sTopoFile.begin(), m_sTopoFile.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_sSSHCfgFile.begin(), m_sSSHCfgFile.end(), back_inserter(*_data));
    _data->push_back('\0');
}

//----------------------------------------------------------------------

void SSimpleMsgCmd::normalizeToLocal()
{
}

void SSimpleMsgCmd::normalizeToRemote()
{
}

void SSimpleMsgCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "SimpleMsgCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    m_sMsg.assign((string::value_type*)&_data[0]);
}

void SSimpleMsgCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_sMsg.begin(), m_sMsg.end(), back_inserter(*_data));
    _data->push_back('\0');
}

//----------------------------------------------------------------------

void SHostInfoCmd::normalizeToLocal()
{
    m_agentPort = inet::_normalizeRead16(m_agentPort);
    m_agentPid = inet::_normalizeRead32(m_agentPid);
    m_timeStamp = inet::_normalizeRead32(m_timeStamp);
}

void SHostInfoCmd::normalizeToRemote()
{
    m_agentPort = inet::_normalizeWrite16(m_agentPort);
    m_agentPid = inet::_normalizeWrite32(m_agentPid);
    m_timeStamp = inet::_normalizeWrite32(m_timeStamp);
}

void SHostInfoCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "HostInfoCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    size_t idx(0);
    m_agentPort = _data[idx++];
    m_agentPort += (_data[idx] << 8);

    ++idx;
    m_agentPid = _data[idx++];
    m_agentPid += (_data[idx++] << 8);
    m_agentPid += (_data[idx++] << 16);
    m_agentPid += (_data[idx] << 24);

    ++idx;
    m_timeStamp = _data[idx++];
    m_timeStamp += (_data[idx++] << 8);
    m_timeStamp += (_data[idx++] << 16);
    m_timeStamp += (_data[idx] << 24);

    ++idx;
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

    // there are so far only 4 string fields in this msg container
    if (v.size() != 4)
        throw runtime_error("HostInfoCmd: can't import data. Number of fields doesn't match.");

    m_username.assign(v[0]);
    m_host.assign(v[1]);
    m_version.assign(v[2]);
    m_DDSPath.assign(v[3]);
}

void SHostInfoCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_agentPort & 0xFF);
    _data->push_back(m_agentPort >> 8);

    _data->push_back(m_agentPid & 0xFF);
    _data->push_back((m_agentPid >> 8) & 0xFF);
    _data->push_back((m_agentPid >> 16) & 0xFF);
    _data->push_back((m_agentPid >> 24) & 0xFF);

    _data->push_back(m_timeStamp & 0xFF);
    _data->push_back((m_timeStamp >> 8) & 0xFF);
    _data->push_back((m_timeStamp >> 16) & 0xFF);
    _data->push_back((m_timeStamp >> 24) & 0xFF);

    copy(m_username.begin(), m_username.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_host.begin(), m_host.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_version.begin(), m_version.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_DDSPath.begin(), m_DDSPath.end(), back_inserter(*_data));
    _data->push_back('\0');
}

//----------------------------------------------------------------------

void SIdCmd::normalizeToLocal()
{
    m_id = inet::_normalizeRead32(m_id);
}

void SIdCmd::normalizeToRemote()
{
    m_id = inet::_normalizeWrite32(m_id);
}

void SIdCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "IdCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    m_id = _data[0];
    m_id += (_data[1] << 8);
    m_id += (_data[2] << 16);
    m_id += (_data[3] << 24);
}

void SIdCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_id & 0xFF);
    _data->push_back((m_id >> 8) & 0xFF);
    _data->push_back((m_id >> 16) & 0xFF);
    _data->push_back((m_id >> 24) & 0xFF);
}

//----------------------------------------------------------------------

void SWnListCmd::normalizeToLocal()
{
}

void SWnListCmd::normalizeToRemote()
{
}

void SWnListCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    m_container.clear();

    size_t idx(0);
    MiscCommon::BYTEVector_t::const_iterator iter = _data.begin();
    MiscCommon::BYTEVector_t::const_iterator iter_end = _data.end();
    string tmp_str;
    for (; iter != iter_end; ++iter, ++idx)
    {
        char c(*iter);
        if ('\0' == c)
        {
            m_container.push_back(tmp_str);
            tmp_str.clear();
            continue;
        }
        tmp_str.push_back(c);
    }

    if (_data.size() < size())
    {
        stringstream ss;
        ss << "WnListCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }
}

void SWnListCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    MiscCommon::StringVector_t::const_iterator iter = m_container.begin();
    MiscCommon::StringVector_t::const_iterator iter_end = m_container.end();
    for (; iter != iter_end; ++iter)
    {
        copy(iter->begin(), iter->end(), back_inserter(*_data));
        _data->push_back('\0');
    }
}

//----------------------------------------------------------------------

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

//----------------------------------------------------------------------

void SBinaryDownloadStatCmd::normalizeToLocal()
{
    m_recievedFileSize = inet::_normalizeRead32(m_recievedFileSize);
    m_recievedCrc32 = inet::_normalizeRead32(m_recievedCrc32);
    m_downloadTime = inet::_normalizeRead32(m_downloadTime);
}

void SBinaryDownloadStatCmd::normalizeToRemote()
{
    m_recievedFileSize = inet::_normalizeWrite32(m_recievedFileSize);
    m_recievedCrc32 = inet::_normalizeWrite32(m_recievedCrc32);
    m_downloadTime = inet::_normalizeWrite32(m_downloadTime);
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
}

//----------------------------------------------------------------------

void SUUIDCmd::normalizeToLocal()
{
}

void SUUIDCmd::normalizeToRemote()
{
}

void SUUIDCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    copy(_data.begin(), _data.end(), m_id.begin());
}

void SUUIDCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_id.begin(), m_id.end(), back_inserter(*_data));
}

//----------------------------------------------------------------------

void SAgentsInfoCmd::normalizeToLocal()
{
    m_nActiveAgents = inet::_normalizeRead16(m_nActiveAgents);
}

void SAgentsInfoCmd::normalizeToRemote()
{
    m_nActiveAgents = inet::_normalizeWrite16(m_nActiveAgents);
}

void SAgentsInfoCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "AgentsInfoCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    size_t idx(0);
    m_nActiveAgents = _data[idx];
    m_nActiveAgents += (_data[++idx] << 8);

    ++idx;
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

    // there are so far only 1 string fields in this msg container
    if (v.size() != 1)
        throw runtime_error("HostInfoCmd: can't import data. Number of fields doesn't match.");

    m_sListOfAgents.assign(v[0]);
}

void SAgentsInfoCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_nActiveAgents & 0xFF);
    _data->push_back(m_nActiveAgents >> 8);

    copy(m_sListOfAgents.begin(), m_sListOfAgents.end(), back_inserter(*_data));
    _data->push_back('\0');
}
