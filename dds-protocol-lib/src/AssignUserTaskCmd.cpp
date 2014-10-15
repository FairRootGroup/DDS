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
    m_nIsExeAvailableOnWorker = inet::_normalizeRead16(m_nIsExeAvailableOnWorker);
}

void SAssignUserTaskCmd::normalizeToRemote() const
{
    m_nIsExeAvailableOnWorker = inet::_normalizeWrite16(m_nIsExeAvailableOnWorker);
}

void SAssignUserTaskCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    size_t idx(0);
    m_nIsExeAvailableOnWorker = _data[idx++];
    m_nIsExeAvailableOnWorker += (_data[idx] << 8);

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
    size_t nExpecting = 1;
    if (v.size() != nExpecting)
    {
        stringstream ss;
        ss << "SAssignUserTaskCmd: can't import data. Number of fields doesn't match. Expecting " << nExpecting
           << " but received " << v.size();
        throw runtime_error(ss.str());
    }

    m_sExeFile.assign(v[0]);
}

void SAssignUserTaskCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_nIsExeAvailableOnWorker & 0xFF);
    _data->push_back(m_nIsExeAvailableOnWorker >> 8);

    copy(m_sExeFile.begin(), m_sExeFile.end(), back_inserter(*_data));
    _data->push_back('\0');
}
