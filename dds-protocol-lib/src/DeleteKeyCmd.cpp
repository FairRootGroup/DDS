// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "DeleteKeyCmd.h"
#include <stdexcept>

using namespace std;
using namespace dds;
using namespace dds::protocol_api;

void SDeleteKeyCmd::normalizeToLocal() const
{
}

void SDeleteKeyCmd::normalizeToRemote() const
{
}

void SDeleteKeyCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "DeleteKeyCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
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
    if (v.size() != 1)
        throw runtime_error("DeleteKeyCmd: can't import data. Number of fields doesn't match.");

    m_sKey.assign(v[0]);
}

void SDeleteKeyCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_sKey.begin(), m_sKey.end(), back_inserter(*_data));
    _data->push_back('\0');
}
