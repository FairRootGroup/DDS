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
    namespace protocol_api
    {
        struct SSetTopologyCmd : public SBasicCmd<SSetTopologyCmd>
        {
            SSetTopologyCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SSetTopologyCmd& val) const;

            // when 0 - valiadate, any other value - don't validate
            uint16_t m_nDisiableValidation;
            // topology file
            std::string m_sTopologyFile;
        };
        std::ostream& operator<<(std::ostream& _stream, const SSetTopologyCmd& val);
        bool operator!=(const SSetTopologyCmd& lhs, const SSetTopologyCmd& rhs);
    }
}

#endif /* defined(__DDS__SetTopologyCmd__) */
