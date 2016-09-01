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
    namespace protocol_api
    {
        struct SAssignUserTaskCmd : public SBasicCmd<SAssignUserTaskCmd>
        {
            SAssignUserTaskCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SAssignUserTaskCmd& val) const;

            std::string m_sExeFile;
            uint64_t m_taskID;
            uint32_t m_taskIndex;
            uint32_t m_collectionIndex;
            std::string m_taskPath;
            std::string m_groupName;
            std::string m_collectionName;
            std::string m_taskName;
        };
        std::ostream& operator<<(std::ostream& _stream, const SAssignUserTaskCmd& val);
        bool operator!=(const SAssignUserTaskCmd& lhs, const SAssignUserTaskCmd& rhs);
    }
};

#endif /* defined(__DDS__AssignUserTaskCmd__) */
