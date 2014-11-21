// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "AssignUserTaskCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

void SAssignUserTaskCmd::normalizeToLocal() const
{
    m_nID = inet::_normalizeRead32(m_nID);
}

void SAssignUserTaskCmd::normalizeToRemote() const
{
    m_nID = inet::_normalizeWrite32(m_nID);
}

void SAssignUserTaskCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    size_t idx(0);
    m_nID = _data[idx];
    m_nID += (_data[++idx] << 8);
    m_nID += (_data[++idx] << 16);
    m_nID += (_data[++idx] << 24);

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
        throw runtime_error("AssignUserTaskCmd: can't import data. Number of fields doesn't match.");

    m_sExeFile.assign(v[0]);
}

void SAssignUserTaskCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_nID & 0xFF);
    _data->push_back((m_nID >> 8) & 0xFF);
    _data->push_back((m_nID >> 16) & 0xFF);
    _data->push_back((m_nID >> 24) & 0xFF);

    copy(m_sExeFile.begin(), m_sExeFile.end(), back_inserter(*_data));
    _data->push_back('\0');
}
