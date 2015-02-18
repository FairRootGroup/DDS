// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "UpdateKeyCmd.h"
#include <stdexcept>

using namespace std;
using namespace dds;

void SUpdateKeyCmd::normalizeToLocal() const
{
}

void SUpdateKeyCmd::normalizeToRemote() const
{
}

void SUpdateKeyCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "UpdateKeyCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    vector<string> v;
    MiscCommon::BYTEVector_t::const_iterator iter = _data.begin();
    MiscCommon::BYTEVector_t::const_iterator iter_end = _data.end();
    for (; iter != iter_end;)
    {
        string tmp((string::value_type*)(&(*iter)));
        v.push_back(tmp);
        advance(iter, tmp.size() + 1);
    }

    // there are so far only 2 string fields in this msg container
    if (v.size() != 2)
        throw runtime_error("UpdateKeyCmd: can't import data. Number of fields doesn't match.");

    m_sKey.assign(v[0]);
    m_sValue.assign(v[1]);
}

void SUpdateKeyCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_sKey.begin(), m_sKey.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_sValue.begin(), m_sValue.end(), back_inserter(*_data));
    _data->push_back('\0');
}
