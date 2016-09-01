// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "DeleteKeyCmd.h"
#include "INet.h"
#include <stdexcept>

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SDeleteKeyCmd::SDeleteKeyCmd()
    : m_sKey()
{
}

size_t SDeleteKeyCmd::size() const
{
    return dsize(m_sKey);
}

bool SDeleteKeyCmd::operator==(const SDeleteKeyCmd& val) const
{
    return (m_sKey == val.m_sKey);
}

void SDeleteKeyCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_sKey);
}

void SDeleteKeyCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_sKey);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SDeleteKeyCmd& val)
{
    return _stream << "key: " << val.m_sKey;
}

bool dds::protocol_api::operator!=(const SDeleteKeyCmd& lhs, const SDeleteKeyCmd& rhs)
{
    return !(lhs == rhs);
}

void SDeleteKeyCmd::setKey(const string& _propID, uint64_t _taskID)
{
    stringstream ss;
    ss << _propID << "." << _taskID;
    m_sKey = ss.str();
}

string SDeleteKeyCmd::getPropertyID() const
{
    const size_t pos(m_sKey.find_last_of('.'));
    return (pos == string::npos) ? "" : m_sKey.substr(0, pos);
}

uint64_t SDeleteKeyCmd::getTaskID() const
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
