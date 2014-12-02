// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_HandShakeAgentCmd_h
#define DDS_HandShakeAgentCmd_h

// DDS
#include "BasicCmd.h"
// MiscCommon
#include "def.h"

namespace dds
{
    struct SHandShakeAgentCmd : public SBasicCmd<SHandShakeAgentCmd>
    {
        SHandShakeAgentCmd();
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            return (sizeof(m_version) + sizeof(m_submitTime));
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SHandShakeAgentCmd& val) const
        {
            return (m_version == val.m_version && m_submitTime == val.m_submitTime);
        }

        mutable uint16_t m_version;
        // seconds since 1970-01-01 00:00:00 UTC
        mutable uint64_t m_submitTime;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SHandShakeAgentCmd& val)
    {
        return _stream << "protocol version: " << val.m_version << "; startup time: " << val.m_submitTime;
    }
    inline bool operator!=(const SHandShakeAgentCmd& lhs, const SHandShakeAgentCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif
