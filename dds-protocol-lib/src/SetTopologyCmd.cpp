// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "SetTopologyCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;
namespace inet = MiscCommon::INet;

SSetTopologyCmd::SSetTopologyCmd()
    : m_nDisiableValidation(0)
    , m_sTopologyFile()
{
}

size_t SSetTopologyCmd::size() const
{
    return dsize(m_sTopologyFile) + dsize(m_nDisiableValidation);
}

bool SSetTopologyCmd::operator==(const SSetTopologyCmd& val) const
{
    return (m_sTopologyFile == val.m_sTopologyFile && m_nDisiableValidation == val.m_nDisiableValidation);
}

void SSetTopologyCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_nDisiableValidation).get(m_sTopologyFile);
}

void SSetTopologyCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_nDisiableValidation).put(m_sTopologyFile);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SSetTopologyCmd& val)
{
    return _stream << "topo file: " << val.m_sTopologyFile << "; validation "
                   << (val.m_nDisiableValidation ? "disabled" : "enabled");
}
bool dds::protocol_api::operator!=(const SSetTopologyCmd& lhs, const SSetTopologyCmd& rhs)
{
    return !(lhs == rhs);
}