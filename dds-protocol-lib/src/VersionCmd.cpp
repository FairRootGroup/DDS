// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "VersionCmd.h"
// DDS
#include "ProtocolCommands.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SVersionCmd::SVersionCmd()
    : m_version(g_protocolCommandsVersion)
    , m_channelType(0)
{
}

void SVersionCmd::normalizeToLocal() const
{
    m_version = inet::normalizeRead(m_version);
    m_channelType = inet::normalizeRead(m_channelType);
}

void SVersionCmd::normalizeToRemote() const
{
    m_version = inet::normalizeWrite(m_version);
    m_channelType = inet::normalizeWrite(m_channelType);
}

void SVersionCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "VersionCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    size_t idx(0);
    inet::readData(&m_version, &_data, &idx);
    inet::readData(&m_channelType, &_data, &idx);
}

void SVersionCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    inet::pushData(m_version, _data);
    inet::pushData(m_channelType, _data);
}