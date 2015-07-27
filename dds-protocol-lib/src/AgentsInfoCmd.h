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
            SAgentsInfoCmd()
                : m_nActiveAgents(0)
            {
            }
            void normalizeToLocal() const;
            void normalizeToRemote() const;
            size_t size() const
            {
                return sizeof(m_nActiveAgents);
            }
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SAgentsInfoCmd& _val) const
            {
                return (m_nActiveAgents == _val.m_nActiveAgents && m_sListOfAgents == _val.m_sListOfAgents);
            }

            mutable uint16_t m_nActiveAgents;
            std::string m_sListOfAgents;
        };
        inline std::ostream& operator<<(std::ostream& _stream, const SAgentsInfoCmd& _val)
        {
            return _stream << _val.m_nActiveAgents;
        }
        inline bool operator!=(const SAgentsInfoCmd& _lhs, const SAgentsInfoCmd& _rhs)
        {
            return !(_lhs == _rhs);
        }
    }
};

#endif /* defined(__DDS__AgentsInfoCmd__) */
