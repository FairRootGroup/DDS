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
            SCustomCmdCmd()
                : m_senderId(0)
                , m_sCmd()
                , m_sCondition()
            {
            }
            void normalizeToLocal() const;
            void normalizeToRemote() const;
            size_t size() const
            {
                size_t s = sizeof(m_senderId) + (m_sCmd.size() + 1) + (m_sCondition.size() + 1);
                return s;
            }
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SCustomCmdCmd& val) const
            {
                return (m_senderId == val.m_senderId && m_sCmd == val.m_sCmd && m_sCondition == val.m_sCondition);
            }

            mutable uint64_t m_senderId;
            std::string m_sCmd;
            std::string m_sCondition;
        };
        inline std::ostream& operator<<(std::ostream& _stream, const SCustomCmdCmd& val)
        {
            return _stream << "senderId:" << val.m_senderId << "cmd: " << val.m_sCmd
                           << " condition: " << val.m_sCondition;
        }
        inline bool operator!=(const SCustomCmdCmd& lhs, const SCustomCmdCmd& rhs)
        {
            return !(lhs == rhs);
        }
    }
}
#endif
