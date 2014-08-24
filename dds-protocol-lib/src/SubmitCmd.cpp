// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "SubmitCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

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
