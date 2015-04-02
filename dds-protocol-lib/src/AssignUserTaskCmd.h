// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__AssignUserTaskCmd__
#define __DDS__AssignUserTaskCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    struct SAssignUserTaskCmd : public SBasicCmd<SAssignUserTaskCmd>
    {
        SAssignUserTaskCmd()
        {
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            return (m_sExeFile.size() + 1 + m_sID.size() + 1 + sizeof(m_taskIndex) + sizeof(m_collectionIndex));
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SAssignUserTaskCmd& val) const
        {
            return (m_sExeFile == val.m_sExeFile && m_sID == val.m_sID && m_taskIndex == val.m_taskIndex &&
                    m_collectionIndex == val.m_collectionIndex);
        }

        std::string m_sExeFile;
        std::string m_sID;
        mutable uint32_t m_taskIndex;
        mutable uint32_t m_collectionIndex;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SAssignUserTaskCmd& val)
    {
        return _stream << "TaskId: " << val.m_sID << "; Exe: " << val.m_sExeFile << "; taskIndex:" << val.m_taskIndex
                       << "; collectionIndex:" << val.m_collectionIndex;
    }
    inline bool operator!=(const SAssignUserTaskCmd& lhs, const SAssignUserTaskCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__AssignUserTaskCmd__) */
