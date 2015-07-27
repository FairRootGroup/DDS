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
            SUpdateKeyCmd()
            {
            }
            void normalizeToLocal() const;
            void normalizeToRemote() const;
            size_t size() const
            {
                size_t s = (m_sKey.size() + 1) + (m_sValue.size() + 1);
                return s;
            }
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SUpdateKeyCmd& val) const
            {
                return (m_sKey == val.m_sKey && m_sValue == val.m_sValue);
            }

            std::string m_sKey;
            std::string m_sValue;
        };
        inline std::ostream& operator<<(std::ostream& _stream, const SUpdateKeyCmd& val)
        {
            return _stream << "key: " << val.m_sKey << " Value: " << val.m_sValue;
        }
        inline bool operator!=(const SUpdateKeyCmd& lhs, const SUpdateKeyCmd& rhs)
        {
            return !(lhs == rhs);
        }
    }
}
#endif
