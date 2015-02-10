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
    m_agentPort = inet::normalizeRead(m_agentPort);
    m_agentPid = inet::normalizeRead(m_agentPid);
    m_submitTime = inet::normalizeRead(m_submitTime);
}

void SHostInfoCmd::normalizeToRemote() const
{
    m_agentPort = inet::normalizeWrite(m_agentPort);
    m_agentPid = inet::normalizeWrite(m_agentPid);
    m_submitTime = inet::normalizeWrite(m_submitTime);
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
    inet::readData(&m_agentPort, &_data, &idx);
    inet::readData(&m_agentPid, &_data, &idx);
    inet::readData(&m_submitTime, &_data, &idx);

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
    inet::pushData(m_agentPort, _data);
    inet::pushData(m_agentPid, _data);
    inet::pushData(m_submitTime, _data);

    copy(m_username.begin(), m_username.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_host.begin(), m_host.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_version.begin(), m_version.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_DDSPath.begin(), m_DDSPath.end(), back_inserter(*_data));
    _data->push_back('\0');
}
