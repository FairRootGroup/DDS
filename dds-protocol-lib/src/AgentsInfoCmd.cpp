// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "AgentsInfoCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

void SAgentsInfoCmd::normalizeToLocal() const
{
    m_nActiveAgents = inet::_normalizeRead16(m_nActiveAgents);
}

void SAgentsInfoCmd::normalizeToRemote() const
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
