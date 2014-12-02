// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "HandShakeAgentCmd.h"
// DDS
#include "ProtocolCommands.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

SHandShakeAgentCmd::SHandShakeAgentCmd()
    : m_version(g_protocolCommandsVersion)
    , m_submitTime(0)
{
}

void SHandShakeAgentCmd::normalizeToLocal() const
{
    m_version = inet::_normalizeRead16(m_version);
    m_submitTime = inet::_normalizeRead64(m_submitTime);
}

void SHandShakeAgentCmd::normalizeToRemote() const
{
    m_version = inet::_normalizeWrite16(m_version);
    m_submitTime = inet::_normalizeWrite64(m_submitTime);
}

void SHandShakeAgentCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "HandShakeAgentCmd: Protocol message data is too short, expected " << size() << " received "
           << _data.size();
        throw runtime_error(ss.str());
    }

    size_t idx(0);
    m_version = _data[idx++];
    m_version += (_data[idx] << 8);

    ++idx;
    m_submitTime = _data[idx++];
    m_submitTime += ((uint64_t)_data[idx++] << 8);
    m_submitTime += ((uint64_t)_data[idx++] << 16);
    m_submitTime += ((uint64_t)_data[idx++] << 24);
    m_submitTime += ((uint64_t)_data[idx++] << 32);
    m_submitTime += ((uint64_t)_data[idx++] << 40);
    m_submitTime += ((uint64_t)_data[idx++] << 48);
    m_submitTime += ((uint64_t)_data[idx++] << 56);
}

void SHandShakeAgentCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_version & 0xFF);
    _data->push_back(m_version >> 8);

    _data->push_back(m_submitTime & 0xFF);
    _data->push_back((m_submitTime >> 8) & 0xFF);
    _data->push_back((m_submitTime >> 16) & 0xFF);
    _data->push_back((m_submitTime >> 24) & 0xFF);
    _data->push_back((m_submitTime >> 32) & 0xFF);
    _data->push_back((m_submitTime >> 40) & 0xFF);
    _data->push_back((m_submitTime >> 48) & 0xFF);
    _data->push_back((m_submitTime >> 56) & 0xFF);
}