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
        // the message is sent for each agent
        struct SAgentsInfoCmd : public SBasicCmd<SAgentsInfoCmd>
        {
            SAgentsInfoCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SAgentsInfoCmd& _val) const;

            uint32_t m_nActiveAgents; /// the number of online agents
            uint32_t m_nIndex;        /// index of the current agent
            std::string m_sAgentInfo; /// info on the current agent
        };
        std::ostream& operator<<(std::ostream& _stream, const SAgentsInfoCmd& _val);
        bool operator!=(const SAgentsInfoCmd& _lhs, const SAgentsInfoCmd& _rhs);
    } // namespace protocol_api
};    // namespace dds

#endif /* defined(__DDS__AgentsInfoCmd__) */
