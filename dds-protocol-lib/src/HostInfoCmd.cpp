// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "HostInfoCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

void SHostInfoCmd::normalizeToLocal() const
{
    m_agentPort = inet::_normalizeRead16(m_agentPort);
    m_agentPid = inet::_normalizeRead32(m_agentPid);
    m_submitTime = inet::_normalizeRead64(m_submitTime);
}

void SHostInfoCmd::normalizeToRemote() const
{
    m_agentPort = inet::_normalizeWrite16(m_agentPort);
    m_agentPid = inet::_normalizeWrite32(m_agentPid);
    m_submitTime = inet::_normalizeWrite64(m_submitTime);
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
    m_submitTime = _data[idx++];
    m_submitTime += ((uint64_t)_data[idx++] << 8);
    m_submitTime += ((uint64_t)_data[idx++] << 16);
    m_submitTime += ((uint64_t)_data[idx++] << 24);
    m_submitTime += ((uint64_t)_data[idx++] << 32);
    m_submitTime += ((uint64_t)_data[idx++] << 40);
    m_submitTime += ((uint64_t)_data[idx++] << 48);
    m_submitTime += ((uint64_t)_data[idx] << 56);

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

    _data->push_back(m_submitTime & 0xFF);
    _data->push_back((m_submitTime >> 8) & 0xFF);
    _data->push_back((m_submitTime >> 16) & 0xFF);
    _data->push_back((m_submitTime >> 24) & 0xFF);
    _data->push_back((m_submitTime >> 32) & 0xFF);
    _data->push_back((m_submitTime >> 40) & 0xFF);
    _data->push_back((m_submitTime >> 48) & 0xFF);
    _data->push_back((m_submitTime >> 56) & 0xFF);

    copy(m_username.begin(), m_username.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_host.begin(), m_host.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_version.begin(), m_version.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_DDSPath.begin(), m_DDSPath.end(), back_inserter(*_data));
    _data->push_back('\0');
}
