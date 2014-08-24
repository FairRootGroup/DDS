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
    struct SVersionCmd : public SBasicCmd<SVersionCmd>
    {
        SVersionCmd();
        void normalizeToLocal();
        void normalizeToRemote();
        size_t size() const
        {
            return sizeof(m_version);
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SVersionCmd& val) const
        {
            return (m_version == val.m_version);
        }

        uint16_t m_version;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SVersionCmd& val)
    {
        return _stream << val.m_version;
    }
    inline bool operator!=(const SVersionCmd& lhs, const SVersionCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__VersionCmd__) */
