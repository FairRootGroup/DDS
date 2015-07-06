// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "UUIDCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

void SIDCmd::normalizeToLocal() const
{
    m_id = inet::normalizeRead(m_id);
}

void SIDCmd::normalizeToRemote() const
{
    m_id = inet::normalizeWrite(m_id);
}

void SIDCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "SIDCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    size_t idx(0);
    inet::readData(&m_id, &_data, &idx);
}

void SIDCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    inet::pushData(m_id, _data);
}
