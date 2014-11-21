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
}

void SAssignUserTaskCmd::normalizeToRemote() const
{
}

void SAssignUserTaskCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
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
        throw runtime_error("AssignUserTaskCmd: can't import data. Number of fields doesn't match.");

    m_sExeFile.assign(v[0]);
    m_sID.assign(v[1]);
}

void SAssignUserTaskCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_sExeFile.begin(), m_sExeFile.end(), back_inserter(*_data));
    _data->push_back('\0');

    copy(m_sID.begin(), m_sID.end(), back_inserter(*_data));
    _data->push_back('\0');
}
