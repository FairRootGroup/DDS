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

SUserTaskDoneCmd::SUserTaskDoneCmd()
    : m_exitCode(0)
    , m_taskID(0)
{
}

size_t SUserTaskDoneCmd::size() const
{
    return dsize(m_exitCode) + dsize(m_taskID);
}

bool SUserTaskDoneCmd::operator==(const SUserTaskDoneCmd& val) const
{
    return (m_exitCode == val.m_exitCode) && (m_taskID == val.m_taskID);
}

void SUserTaskDoneCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_exitCode).get(m_taskID);
}

void SUserTaskDoneCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_exitCode).put(m_taskID);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SUserTaskDoneCmd& val)
{
    return _stream << "exit code: " << val.m_exitCode << " taskID: " << val.m_taskID;
}

bool dds::protocol_api::operator!=(const SUserTaskDoneCmd& lhs, const SUserTaskDoneCmd& rhs)
{
    return !(lhs == rhs);
}
