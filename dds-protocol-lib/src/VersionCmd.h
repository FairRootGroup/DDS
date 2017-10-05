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
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SVersionCmd& val) const;

            std::string m_sSID; /// Session ID
            uint16_t m_version;
            uint16_t m_channelType;
        };
        std::ostream& operator<<(std::ostream& _stream, const SVersionCmd& val);
        bool operator!=(const SVersionCmd& lhs, const SVersionCmd& rhs);
    }
}

#endif /* defined(__DDS__VersionCmd__) */
