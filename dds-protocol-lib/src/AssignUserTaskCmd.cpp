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

void SAssignUserTaskCmd::normalizeToLocal()
{
}

void SAssignUserTaskCmd::normalizeToRemote()
{
}

void SAssignUserTaskCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "SAssignUserTaskCmd: Protocol message data is too short, expected " << size() << " received "
           << _data.size();
        throw runtime_error(ss.str());
    }

    m_sExeFile.assign((string::value_type*)&_data[0]);
}

void SAssignUserTaskCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_sExeFile.begin(), m_sExeFile.end(), back_inserter(*_data));
    _data->push_back('\0');
}
