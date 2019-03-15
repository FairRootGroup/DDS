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
            SGetPropValuesCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SGetPropValuesCmd& val) const;

            std::string m_sPropertyName;
        };
        std::ostream& operator<<(std::ostream& _stream, const SGetPropValuesCmd& val);
        bool operator!=(const SGetPropValuesCmd& lhs, const SGetPropValuesCmd& rhs);
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__GetPropValuesCmd__) */
