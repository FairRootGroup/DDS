// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "UserTaskDoneCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

void SUserTaskDoneCmd::normalizeToLocal() const
{
    m_exitCode = inet::normalizeRead(m_exitCode);
}

void SUserTaskDoneCmd::normalizeToRemote() const
{
    m_exitCode = inet::normalizeWrite(m_exitCode);
}

void SUserTaskDoneCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "SUserTaskDoneCmd: Protocol message data is too short, expected " << size() << " received "
           << _data.size();
        throw runtime_error(ss.str());
    }

    size_t idx(0);
    inet::readData(&m_exitCode, &_data, &idx);
}

void SUserTaskDoneCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    inet::pushData(m_exitCode, _data);
}
