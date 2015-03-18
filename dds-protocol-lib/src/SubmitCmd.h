// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__SubmitCmd__
#define __DDS__SubmitCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    struct SSubmitCmd : public SBasicCmd<SSubmitCmd>
    {
        // a list of supported RMS
        // We define this enum here to make a strong binding between protocol and RMS type code.
        // Just in case if code of any of the supported RMS is changed (enum was re-ordered), then protocol version
        // should also be changed.
        enum ERmsType
        {
            UNKNOWN = -1,
            SSH = 0
        };
        std::map<uint16_t, std::string> RMSTypeCodeToString = { { SSH, "ssh" } };

        SSubmitCmd()
            : m_nRMSTypeCode(0)
        {
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            size_t s = (m_sSSHCfgFile.size() + 1) + sizeof(m_nRMSTypeCode);
            return s;
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SSubmitCmd& val) const
        {
            return (m_sSSHCfgFile == val.m_sSSHCfgFile && m_nRMSTypeCode == val.m_nRMSTypeCode);
        }

        mutable uint16_t m_nRMSTypeCode;
        std::string m_sSSHCfgFile;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SSubmitCmd& val)
    {
        return _stream << "RMS type code: " << val.m_nRMSTypeCode << "; SSH Hosts config: " << val.m_sSSHCfgFile;
    }
    inline bool operator!=(const SSubmitCmd& lhs, const SSubmitCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__SubmitCmd__) */
