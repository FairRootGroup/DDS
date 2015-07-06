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
            SIDCmd()
                : m_id()
            {
            }
            void normalizeToLocal() const;
            void normalizeToRemote() const;
            size_t size() const
            {
                size_t size(sizeof(m_id));
                return size;
            }
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SIDCmd& _val) const
            {
                return (m_id == _val.m_id);
            }

            mutable uint64_t m_id;
        };
        inline std::ostream& operator<<(std::ostream& _stream, const SIDCmd& _val)
        {
            _stream << _val.m_id;
            return _stream;
        }
        inline bool operator!=(const SIDCmd& lhs, const SIDCmd& rhs)
        {
            return !(lhs == rhs);
        }
    }
}

#endif /* defined(__DDS__UUIDCmd__) */
