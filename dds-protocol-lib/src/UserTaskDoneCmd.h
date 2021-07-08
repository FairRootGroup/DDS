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
    namespace protocol_api
    {
        struct SUserTaskDoneCmd : public SBasicCmd<SUserTaskDoneCmd>
        {
            SUserTaskDoneCmd();
            size_t size() const;
            void _convertFromData(const dds::misc::BYTEVector_t& _data);
            void _convertToData(dds::misc::BYTEVector_t* _data) const;
            bool operator==(const SUserTaskDoneCmd& val) const;

            uint32_t m_exitCode;
            uint64_t m_taskID;
        };
        std::ostream& operator<<(std::ostream& _stream, const SUserTaskDoneCmd& val);
        bool operator!=(const SUserTaskDoneCmd& lhs, const SUserTaskDoneCmd& rhs);
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__UserTaskDoneCmd__) */
