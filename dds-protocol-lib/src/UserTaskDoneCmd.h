// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__UserTaskDoneCmd__
#define __DDS__UserTaskDoneCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    struct SUserTaskDoneCmd : public SBasicCmd<SUserTaskDoneCmd>
    {
        SUserTaskDoneCmd()
            : m_exitCode(0)
        {
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            return sizeof(m_exitCode);
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SUserTaskDoneCmd& val) const
        {
            return (m_exitCode == val.m_exitCode);
        }

        mutable uint32_t m_exitCode;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SUserTaskDoneCmd& val)
    {
        return _stream << "exit code: " << val.m_exitCode;
    }
    inline bool operator!=(const SUserTaskDoneCmd& lhs, const SUserTaskDoneCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__UserTaskDoneCmd__) */
