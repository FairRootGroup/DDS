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
            , m_timeStamp(0)
        {
        }
        size_t size() const
        {
            size_t size(m_username.size() + 1);
            size += m_host.size() + 1;
            size += sizeof(m_agentPort);
            size += sizeof(m_agentPid);
            size += m_version.size() + 1;
            size += m_DDSPath.size() + 1;
            size += sizeof(m_timeStamp);
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
                    m_timeStamp == val.m_timeStamp);
        }

        mutable uint16_t m_agentPort;
        mutable uint32_t m_agentPid;
        mutable uint32_t m_timeStamp; // defines a time stamp when DDS Job was submitted
        std::string m_username;
        std::string m_host;
        std::string m_version;
        std::string m_DDSPath;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SHostInfoCmd& val)
    {
        _stream << val.m_username << ":" << val.m_host << ": " << val.m_version << ":" << val.m_DDSPath << "; agent ["
                << val.m_agentPid << "] on port " << val.m_agentPort << "; submitted on " << val.m_timeStamp;
        return _stream;
    }
    inline bool operator!=(const SHostInfoCmd& lhs, const SHostInfoCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__HostInfoCmd__) */
