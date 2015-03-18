// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__SetTopologyCmd__
#define __DDS__SetTopologyCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    struct SSetTopologyCmd : public SBasicCmd<SSetTopologyCmd>
    {
        SSetTopologyCmd()
            : m_nDisiableValidation(0)
        {
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            return (m_sTopologyFile.size() + 1) + sizeof(m_nDisiableValidation);
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SSetTopologyCmd& val) const
        {
            return (m_sTopologyFile == val.m_sTopologyFile && m_nDisiableValidation == val.m_nDisiableValidation);
        }

        // when 0 - valiadate, any other value - don't validate
        mutable uint16_t m_nDisiableValidation;
        // topology file
        std::string m_sTopologyFile;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SSetTopologyCmd& val)
    {
        return _stream << "topo file: " << val.m_sTopologyFile << "; validation "
                       << (val.m_nDisiableValidation ? "disabled" : "enabled");
    }
    inline bool operator!=(const SSetTopologyCmd& lhs, const SSetTopologyCmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__SetTopologyCmd__) */
