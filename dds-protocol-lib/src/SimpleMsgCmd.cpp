// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "SimpleMsgCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

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

    size_t idx(0);
    m_msgSeverity = _data[idx++];
    m_msgSeverity += (_data[idx] << 8);

    ++idx;
    m_srcCommand = _data[idx++];
    m_srcCommand += (_data[idx] << 8);

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
        throw runtime_error("SimpleMsgCmd: can't import data. Number of fields doesn't match.");

    m_sMsg.assign(v[0]);
}

void SSimpleMsgCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_msgSeverity & 0xFF);
    _data->push_back(m_msgSeverity >> 8);

    _data->push_back(m_srcCommand & 0xFF);
    _data->push_back(m_srcCommand >> 8);

    copy(m_sMsg.begin(), m_sMsg.end(), back_inserter(*_data));
    _data->push_back('\0');
}
