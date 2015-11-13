// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "CustomCmdCmd.h"
#include <stdexcept>
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

void SCustomCmdCmd::normalizeToLocal() const
{
    m_senderId = inet::normalizeRead(m_senderId);
}

void SCustomCmdCmd::normalizeToRemote() const
{
    m_senderId = inet::normalizeWrite(m_senderId);
}

void SCustomCmdCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "CustomCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    size_t idx(0);
    inet::readData(&m_senderId, &_data, &idx);

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
        throw runtime_error("CustomCmd: can't import data. Number of fields doesn't match.");

    m_sCmd.assign(v[0]);
    m_sCondition.assign(v[1]);
}

void SCustomCmdCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    inet::pushData(m_senderId, _data);

    copy(m_sCmd.begin(), m_sCmd.end(), back_inserter(*_data));
    _data->push_back('\0');
    copy(m_sCondition.begin(), m_sCondition.end(), back_inserter(*_data));
    _data->push_back('\0');
}
