// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__UUIDCmd__
#define __DDS__UUIDCmd__
// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SIDCmd : public SBasicCmd<SIDCmd>
        {
            SIDCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SIDCmd& _val) const;

            uint64_t m_id;
        };
        std::ostream& operator<<(std::ostream& _stream, const SIDCmd& _val);
        bool operator!=(const SIDCmd& lhs, const SIDCmd& rhs);
    }
}

#endif /* defined(__DDS__UUIDCmd__) */
