// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "AgentsInfoCmd.h"

using namespace std;
using namespace dds;
using namespace MiscCommon;
using namespace dds::protocol_api;

SAgentsInfoCmd::SAgentsInfoCmd()
    : m_nActiveAgents(0)
    , m_nIndex(0)
{
}

size_t SAgentsInfoCmd::size() const
{
    return dsize(m_nActiveAgents) + dsize(m_nIndex) + dsize(m_sAgentInfo);
}

bool SAgentsInfoCmd::operator==(const SAgentsInfoCmd& _val) const
{
    return (m_nActiveAgents == _val.m_nActiveAgents && m_nIndex == _val.m_nIndex && m_sAgentInfo == _val.m_sAgentInfo);
}

void SAgentsInfoCmd::_convertFromData(const BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_nActiveAgents).get(m_nIndex).get(m_sAgentInfo);
}

void SAgentsInfoCmd::_convertToData(BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_nActiveAgents).put(m_nIndex).put(m_sAgentInfo);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SAgentsInfoCmd& _val)
{
    return _stream << _val.m_nActiveAgents;
}

bool dds::protocol_api::operator!=(const SAgentsInfoCmd& _lhs, const SAgentsInfoCmd& _rhs)
{
    return !(_lhs == _rhs);
}
