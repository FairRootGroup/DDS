// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DeleteKeyCmd__
#define __DDS__DeleteKeyCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    struct SDeleteKeyCmd : public SBasicCmd<SDeleteKeyCmd>
    {
        SDeleteKeyCmd()
        {
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            size_t s = (m_sKey.size() + 1);
            return s;
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SDeleteKeyCmd& val) const
        {
            return (m_sKey == val.m_sKey);
        }

        std::string m_sKey;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SDeleteKeyCmd& val)
    {
        return _stream << "key: " << val.m_sKey;
    }
    inline bool operator!=(const SDeleteKeyCmd& lhs, const SDeleteKeyCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__DeleteKeyCmd__) */
