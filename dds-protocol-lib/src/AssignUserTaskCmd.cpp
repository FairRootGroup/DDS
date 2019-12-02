// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "AssignUserTaskCmd.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;

SAssignUserTaskCmd::SAssignUserTaskCmd()
    : m_sExeFile()
    , m_taskID(0)
    , m_taskIndex(0)
    , m_collectionIndex(0)
    , m_taskPath()
    , m_groupName()
    , m_collectionName()
    , m_taskName()
    , m_topoHash(0)
{
}

size_t SAssignUserTaskCmd::size() const
{
    return dsize(m_sExeFile) + dsize(m_taskID) + dsize(m_taskIndex) + dsize(m_collectionIndex) + dsize(m_taskPath) +
           dsize(m_groupName) + dsize(m_collectionName) + dsize(m_taskName) + dsize(m_topoHash);
}

bool SAssignUserTaskCmd::operator==(const SAssignUserTaskCmd& val) const
{
    return (m_sExeFile == val.m_sExeFile && m_taskID == val.m_taskID && m_taskIndex == val.m_taskIndex &&
            m_collectionIndex == val.m_collectionIndex && m_taskPath == val.m_taskPath &&
            m_groupName == val.m_groupName && m_collectionName == val.m_collectionName &&
            m_taskName == val.m_taskName && m_topoHash == val.m_topoHash);
}

void SAssignUserTaskCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data)
        .get(m_taskIndex)
        .get(m_collectionIndex)
        .get(m_sExeFile)
        .get(m_taskID)
        .get(m_taskPath)
        .get(m_groupName)
        .get(m_collectionName)
        .get(m_taskName)
        .get(m_topoHash);
}

void SAssignUserTaskCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data)
        .put(m_taskIndex)
        .put(m_collectionIndex)
        .put(m_sExeFile)
        .put(m_taskID)
        .put(m_taskPath)
        .put(m_groupName)
        .put(m_collectionName)
        .put(m_taskName)
        .put(m_topoHash);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SAssignUserTaskCmd& val)
{
    return _stream << "TaskId: " << val.m_taskID << "; Exe: " << val.m_sExeFile << "; taskIndex:" << val.m_taskIndex
                   << "; collectionIndex:" << val.m_collectionIndex << "; taskPath:" << val.m_taskPath
                   << "; groupName:" << val.m_groupName << "; collectionName:" << val.m_collectionName
                   << "; taskName: " << val.m_taskName << "; topoHash: " << val.m_topoHash;
}

bool dds::protocol_api::operator!=(const SAssignUserTaskCmd& lhs, const SAssignUserTaskCmd& rhs)
{
    return !(lhs == rhs);
}
