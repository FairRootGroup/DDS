// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "UpdateKeyCmd.h"
#include "INet.h"
#include <stdexcept>

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SUpdateKeyCmd::SUpdateKeyCmd()
{
}

size_t SUpdateKeyCmd::size() const
{
    return dsize(m_sKey) + dsize(m_sValue);
}

bool SUpdateKeyCmd::operator==(const SUpdateKeyCmd& val) const
{
    return (m_sKey == val.m_sKey && m_sValue == val.m_sValue);
}

void SUpdateKeyCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_sKey).get(m_sValue);
}

void SUpdateKeyCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_sKey).put(m_sValue);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SUpdateKeyCmd& val)
{
    return _stream << "key: " << val.m_sKey << " value: " << val.m_sValue;
}

bool dds::protocol_api::operator!=(const SUpdateKeyCmd& lhs, const SUpdateKeyCmd& rhs)
{
    return !(lhs == rhs);
}

void SUpdateKeyCmd::setKey(const string& _propID, uint64_t _taskID)
{
    stringstream ss;
    ss << _propID << "." << _taskID;
    m_sKey = ss.str();
}

string SUpdateKeyCmd::getPropertyID() const
{
    const size_t pos(m_sKey.find_last_of('.'));
    return (pos == string::npos) ? "" : m_sKey.substr(0, pos);
}

uint64_t SUpdateKeyCmd::getTaskID() const
{
    const size_t pos(m_sKey.find_last_of('.'));
    if (pos == string::npos)
    {
        return 0;
    }
    try
    {
        return std::stoull(m_sKey.substr(pos + 1));
    }
    catch (exception& _e)
    {
        return 0;
    }
}
