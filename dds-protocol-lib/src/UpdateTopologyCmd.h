// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__UpdateTopologyCmd__
#define __DDS__UpdateTopologyCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SUpdateTopologyCmd : public SBasicCmd<SUpdateTopologyCmd>
        {
            enum class EUpdateType : uint8_t
            {
                UPDATE = 0,
                ACTIVATE,
                STOP
            };

            SUpdateTopologyCmd();
            size_t size() const;
            void _convertFromData(const dds::misc::BYTEVector_t& _data);
            void _convertToData(dds::misc::BYTEVector_t* _data) const;
            bool operator==(const SUpdateTopologyCmd& val) const;

            // when 0 - validate, any other value - don't validate
            uint16_t m_nDisableValidation;
            // topology file
            std::string m_sTopologyFile;
            // topology update type
            uint8_t m_updateType;
        };
        std::ostream& operator<<(std::ostream& _stream, const SUpdateTopologyCmd& val);
        bool operator!=(const SUpdateTopologyCmd& lhs, const SUpdateTopologyCmd& rhs);
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__UpdateTopologyCmd__) */
