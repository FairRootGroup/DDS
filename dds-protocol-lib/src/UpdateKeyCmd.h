// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_UpdateKeyCmd_h
#define DDS_UpdateKeyCmd_h
// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SUpdateKeyCmd : public SBasicCmd<SUpdateKeyCmd>
        {

            SUpdateKeyCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SUpdateKeyCmd& val) const;
            
            std::string m_propertyID;
            std::string m_value;
            uint64_t m_senderTaskID;
            uint64_t m_receiverTaskID;
        };
        std::ostream& operator<<(std::ostream& _stream, const SUpdateKeyCmd& val);
        bool operator!=(const SUpdateKeyCmd& lhs, const SUpdateKeyCmd& rhs);
    } // namespace protocol_api
} // namespace dds
#endif
