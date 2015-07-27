// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__VersionCmd__
#define __DDS__VersionCmd__

// DDS
#include "BasicCmd.h"
// MiscCommon
#include "def.h"

namespace dds
{
    namespace protocol_api
    {
        struct SVersionCmd : public SBasicCmd<SVersionCmd>
        {
            SVersionCmd();
            void normalizeToLocal() const;
            void normalizeToRemote() const;
            size_t size() const
            {
                return sizeof(m_version) + sizeof(m_channelType);
            }
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SVersionCmd& val) const
            {
                return (m_version == val.m_version) && (m_channelType == val.m_channelType);
            }

            mutable uint16_t m_version;
            mutable uint16_t m_channelType;
        };
        inline std::ostream& operator<<(std::ostream& _stream, const SVersionCmd& val)
        {
            return _stream << val.m_version << " " << val.m_channelType;
        }
        inline bool operator!=(const SVersionCmd& lhs, const SVersionCmd& rhs)
        {
            return !(lhs == rhs);
        }
    }
}

#endif /* defined(__DDS__VersionCmd__) */
