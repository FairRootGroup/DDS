// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__SimpleMsgCmd__
#define __DDS__SimpleMsgCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    struct SSimpleMsgCmd : public SBasicCmd<SSimpleMsgCmd>
    {
        SSimpleMsgCmd()
            : m_msgSeverity(0)
            , m_srcCommand(0)
        {
        }
        void normalizeToLocal();
        void normalizeToRemote();
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

        uint16_t m_msgSeverity;
        uint16_t m_srcCommand;
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
};

#endif /* defined(__DDS__SimpleMsgCmd__) */
