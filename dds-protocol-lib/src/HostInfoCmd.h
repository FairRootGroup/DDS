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
    struct SHostInfoCmd : public SBasicCmd<SHostInfoCmd>
    {
        SHostInfoCmd()
            : m_agentPort(0)
            , m_agentPid(0)
            , m_submitTime(0)
        {
        }
        size_t size() const
        {
            size_t size(m_username.size() + 1);
            size += m_host.size() + 1;
            size += sizeof(m_agentPort);
            size += sizeof(m_agentPid);
            size += sizeof(m_submitTime);
            size += m_version.size() + 1;
            size += m_DDSPath.size() + 1;
            return size;
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SHostInfoCmd& val) const
        {
            return (m_username == val.m_username && m_host == val.m_host && m_version == val.m_version &&
                    m_DDSPath == val.m_DDSPath && m_agentPort == val.m_agentPort && m_agentPid == val.m_agentPid &&
                    m_submitTime == val.m_submitTime);
        }

        mutable uint16_t m_agentPort;
        mutable uint32_t m_agentPid;
        // milliseconds since 1970-01-01 00:00:00 UTC
        mutable uint64_t m_submitTime;
        std::string m_username;
        std::string m_host;
        std::string m_version;
        std::string m_DDSPath;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SHostInfoCmd& val)
    {
        _stream << val.m_username << ":" << val.m_host << ": " << val.m_version << ":" << val.m_DDSPath << "; agent ["
                << val.m_agentPid << "] on port " << val.m_agentPort << "; startup time: " << val.m_submitTime;
        return _stream;
    }
    inline bool operator!=(const SHostInfoCmd& lhs, const SHostInfoCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__HostInfoCmd__) */
