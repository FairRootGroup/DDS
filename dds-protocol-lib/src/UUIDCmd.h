// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__UUIDCmd__
#define __DDS__UUIDCmd__
// DDS
#include "BasicCmd.h"
// BOOST
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#pragma clang diagnostic pop

namespace dds
{
    namespace protocol_api
    {
        struct SUUIDCmd : public SBasicCmd<SUUIDCmd>
        {
            SUUIDCmd()
                : m_id()
            {
            }
            void normalizeToLocal() const;
            void normalizeToRemote() const;
            size_t size() const
            {
                size_t size(boost::uuids::uuid::static_size());
                return size;
            }
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SUUIDCmd& _val) const
            {
                return (m_id == _val.m_id);
            }

            boost::uuids::uuid m_id;
        };
        inline std::ostream& operator<<(std::ostream& _stream, const SUUIDCmd& _val)
        {
            _stream << _val.m_id;
            return _stream;
        }
    }
}

#endif /* defined(__DDS__UUIDCmd__) */
