// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_CustomCmdCmd_h
#define DDS_CustomCmdCmd_h
// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SCustomCmdCmd : public SBasicCmd<SCustomCmdCmd>
        {
            SCustomCmdCmd();
            size_t size() const;
            void _convertFromData(const dds::misc::BYTEVector_t& _data);
            void _convertToData(dds::misc::BYTEVector_t* _data) const;
            bool operator==(const SCustomCmdCmd& val) const;

            uint64_t m_timestamp;
            uint64_t m_senderId;
            std::string m_sCmd;
            std::string m_sCondition;
        };
        std::ostream& operator<<(std::ostream& _stream, const SCustomCmdCmd& val);
        bool operator!=(const SCustomCmdCmd& lhs, const SCustomCmdCmd& rhs);
    } // namespace protocol_api
} // namespace dds
#endif
