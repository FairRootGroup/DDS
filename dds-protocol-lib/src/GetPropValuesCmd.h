// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__GetPropValuesCmd__
#define __DDS__GetPropValuesCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SGetPropValuesCmd : public SBasicCmd<SGetPropValuesCmd>
        {
            SGetPropValuesCmd()
            {
            }
            void normalizeToLocal() const;
            void normalizeToRemote() const;
            size_t size() const
            {
                size_t s = (m_sPropertyID.size() + 1);
                return s;
            }
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SGetPropValuesCmd& val) const
            {
                return (m_sPropertyID == val.m_sPropertyID);
            }

            std::string m_sPropertyID;
        };
        inline std::ostream& operator<<(std::ostream& _stream, const SGetPropValuesCmd& val)
        {
            return _stream << "propertyID: " << val.m_sPropertyID;
        }
        inline bool operator!=(const SGetPropValuesCmd& lhs, const SGetPropValuesCmd& rhs)
        {
            return !(lhs == rhs);
        }
    }
}

#endif /* defined(__DDS__GetPropValuesCmd__) */
