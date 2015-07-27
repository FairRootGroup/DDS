// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "SetTopologyCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

void SSetTopologyCmd::normalizeToLocal() const
{
    m_nDisiableValidation = inet::normalizeRead(m_nDisiableValidation);
}

void SSetTopologyCmd::normalizeToRemote() const
{
    m_nDisiableValidation = inet::normalizeWrite(m_nDisiableValidation);
}

void SSetTopologyCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "SSetTopologyCmd: Protocol message data is too short, expected " << size() << " received "
           << _data.size();
        throw runtime_error(ss.str());
    }

    size_t idx(0);
    inet::readData(&m_nDisiableValidation, &_data, &idx);

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
        throw runtime_error("SSetTopologyCmd: can't import data. Number of fields doesn't match.");

    m_sTopologyFile.assign(v[0]);
}

void SSetTopologyCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    inet::pushData(m_nDisiableValidation, _data);

    copy(m_sTopologyFile.begin(), m_sTopologyFile.end(), back_inserter(*_data));
    _data->push_back('\0');
}
