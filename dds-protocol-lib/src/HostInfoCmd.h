// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__HostInfoCmd__
#define __DDS__HostInfoCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SHostInfoCmd : public SBasicCmd<SHostInfoCmd>
        {
            SHostInfoCmd();
            size_t size() const;
            void _convertFromData(const dds::misc::BYTEVector_t& _data);
            void _convertToData(dds::misc::BYTEVector_t* _data) const;
            bool operator==(const SHostInfoCmd& val) const;

            uint32_t m_agentPid;
            // a number of task slots
            uint32_t m_slots;
            // milliseconds since 1970-01-01 00:00:00 UTC
            uint64_t m_submitTime;
            std::string m_username;
            std::string m_host;
            std::string m_version;
            std::string m_DDSPath;
            std::string m_workerId;
        };
        std::ostream& operator<<(std::ostream& _stream, const SHostInfoCmd& val);
        bool operator!=(const SHostInfoCmd& lhs, const SHostInfoCmd& rhs);
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__HostInfoCmd__) */
