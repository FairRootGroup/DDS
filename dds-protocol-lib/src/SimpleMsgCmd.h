// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__SimpleMsgCmd__
#define __DDS__SimpleMsgCmd__

// DDS
#include "BasicCmd.h"
#include "Logger.h"

namespace dds
{
    namespace protocol_api
    {
        struct SSimpleMsgCmd : public SBasicCmd<SSimpleMsgCmd>
        {
            SSimpleMsgCmd()
                : m_msgSeverity(0)
                , m_srcCommand(0)
                , m_sMsg()
            {
            }
            SSimpleMsgCmd(const std::string& _msg, uint16_t _severity = MiscCommon::info, uint16_t _command = 0)
                : m_msgSeverity(_severity)
                , m_srcCommand(_command)
                , m_sMsg(_msg)
            {
            }
            void normalizeToLocal() const;
            void normalizeToRemote() const;
            size_t size() const
            {
                return (m_sMsg.size() + 1) + sizeof(m_msgSeverity) + sizeof(m_srcCommand);
            }
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SSimpleMsgCmd& val) const
            {
                return (m_sMsg == val.m_sMsg && m_msgSeverity == val.m_msgSeverity && m_srcCommand == val.m_srcCommand);
            }

            mutable uint16_t m_msgSeverity;
            mutable uint16_t m_srcCommand;
            std::string m_sMsg;
        };
        inline std::ostream& operator<<(std::ostream& _stream, const SSimpleMsgCmd& val)
        {
            return _stream << "source command: " << val.m_srcCommand << "; severity " << val.m_msgSeverity
                           << "; Msg: " << val.m_sMsg;
        }
        inline bool operator!=(const SSimpleMsgCmd& lhs, const SSimpleMsgCmd& rhs)
        {
            return !(lhs == rhs);
        }
    }
}

#endif /* defined(__DDS__SimpleMsgCmd__) */
