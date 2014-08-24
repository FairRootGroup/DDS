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

    m_sMsg.assign((string::value_type*)&_data[0]);
}

void SSimpleMsgCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_sMsg.begin(), m_sMsg.end(), back_inserter(*_data));
    _data->push_back('\0');
}
