// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__AgentsInfoCmd__
#define __DDS__AgentsInfoCmd__
// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SAgentsInfoCmd : public SBasicCmd<SAgentsInfoCmd>
        {
            SAgentsInfoCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SAgentsInfoCmd& _val) const;

            uint32_t m_nActiveAgents;
            std::string m_sListOfAgents;
        };
        std::ostream& operator<<(std::ostream& _stream, const SAgentsInfoCmd& _val);
        bool operator!=(const SAgentsInfoCmd& _lhs, const SAgentsInfoCmd& _rhs);
    }
};

#endif /* defined(__DDS__AgentsInfoCmd__) */
