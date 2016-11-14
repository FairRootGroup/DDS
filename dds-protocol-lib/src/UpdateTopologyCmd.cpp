// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "UpdateTopologyCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SUpdateTopologyCmd::SUpdateTopologyCmd()
    : m_nDisiableValidation(0)
    , m_sTopologyFile()
    , m_updateType((uint8_t)EUpdateType::UPDATE)
{
}

size_t SUpdateTopologyCmd::size() const
{
    return dsize(m_sTopologyFile) + dsize(m_nDisiableValidation) + dsize(m_updateType);
}

bool SUpdateTopologyCmd::operator==(const SUpdateTopologyCmd& val) const
{
    return (m_sTopologyFile == val.m_sTopologyFile && m_nDisiableValidation == val.m_nDisiableValidation &&
            m_updateType == val.m_updateType);
}

void SUpdateTopologyCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_nDisiableValidation).get(m_sTopologyFile).get(m_updateType);
}

void SUpdateTopologyCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_nDisiableValidation).put(m_sTopologyFile).put(m_updateType);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SUpdateTopologyCmd& val)
{
    return _stream << "topo file: " << val.m_sTopologyFile << "; validation "
                   << (val.m_nDisiableValidation ? "disabled" : "enabled") << "update type: " << val.m_updateType;
}
bool dds::protocol_api::operator!=(const SUpdateTopologyCmd& lhs, const SUpdateTopologyCmd& rhs)
{
    return !(lhs == rhs);
}
