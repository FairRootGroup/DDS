// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "GetPropValuesCmd.h"
#include <stdexcept>

using namespace std;
using namespace dds;

void SGetPropValuesCmd::normalizeToLocal() const
{
}

void SGetPropValuesCmd::normalizeToRemote() const
{
}

void SGetPropValuesCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "GetPropValuesCmd: Protocol message data is too short, expected " << size() << " received "
           << _data.size();
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

    // there are so far only 1 string field1 in this msg container
    if (v.size() != 1)
        throw runtime_error("GetPropValuesCmd: can't import data. Number of fields doesn't match.");

    m_sPropertyID.assign(v[0]);
}

void SGetPropValuesCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_sPropertyID.begin(), m_sPropertyID.end(), back_inserter(*_data));
    _data->push_back('\0');
}