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
            : m_nIsExeAvailableOnWorker(0)
        {
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            return (m_sExeFile.size() + 1 + sizeof(m_nIsExeAvailableOnWorker));
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SAssignUserTaskCmd& val) const
        {
            return (m_sExeFile == val.m_sExeFile && m_nIsExeAvailableOnWorker == val.m_nIsExeAvailableOnWorker);
        }

        mutable uint16_t m_nIsExeAvailableOnWorker;
        std::string m_sExeFile;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SAssignUserTaskCmd& val)
    {
        return _stream << "Exe (" << (val.m_nIsExeAvailableOnWorker ? "local path" : "remote path")
                       << "): " << val.m_sExeFile;
    }
    inline bool operator!=(const SAssignUserTaskCmd& lhs, const SAssignUserTaskCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__AssignUserTaskCmd__) */
