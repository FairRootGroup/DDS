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
        {
        }
        void normalizeToLocal();
        void normalizeToRemote();
        size_t size() const
        {
            return (m_sMsg.size() + 1);
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SSimpleMsgCmd& val) const
        {
            return (m_sMsg == val.m_sMsg);
        }

        std::string m_sMsg;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SSimpleMsgCmd& val)
    {
        return _stream << val.m_sMsg;
    }
    inline bool operator!=(const SSimpleMsgCmd& lhs, const SSimpleMsgCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__SimpleMsgCmd__) */
