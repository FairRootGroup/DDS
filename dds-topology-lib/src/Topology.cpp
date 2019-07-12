// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "Topology.h"
#include "TopoCore.h"

using namespace std;
using namespace dds;
using namespace dds::topology_api;

CTopology::CTopology()
    : CTopology("", "")
{
}

CTopology::CTopology(const std::string& _fileName)
    : CTopology(_fileName, "")
{
}

CTopology::CTopology(const std::string& _fileName, const std::string& _schemaFileName)
    : m_topo(std::make_shared<CTopoCore>())
{
    m_topo->setXMLValidationDisabled(_fileName.empty() || _schemaFileName.empty());
    m_topo->init(_fileName, _schemaFileName);
}

CTopology::~CTopology()
{
}

CTopoGroup::Ptr_t CTopology::getMainGroup() const
{
    return m_topo->getMainGroup();
}

const STopoRuntimeTask& CTopology::getRuntimeTaskById(Id_t _id) const
{
    return m_topo->getRuntimeTaskById(_id);
}

const STopoRuntimeCollection& CTopology::getRuntimeCollectionById(Id_t _id) const
{
    return m_topo->getRuntimeCollectionById(_id);
}

const STopoRuntimeTask& CTopology::getRuntimeTaskByIdPath(const std::string& _idPath) const
{
    return m_topo->getRuntimeTaskByIdPath(_idPath);
}

const STopoRuntimeCollection& CTopology::getRuntimeCollectionByIdPath(const std::string& _idPath) const
{
    return m_topo->getRuntimeCollectionByIdPath(_idPath);
}

STopoRuntimeTask::FilterIteratorPair_t CTopology::getRuntimeTaskIterator(STopoRuntimeTask::Condition_t _condition) const
{
    return m_topo->getRuntimeTaskIterator(_condition);
}

STopoRuntimeCollection::FilterIteratorPair_t CTopology::getRuntimeCollectionIterator(
    STopoRuntimeCollection::Condition_t _condition) const
{
    return m_topo->getRuntimeCollectionIterator(_condition);
}

STopoRuntimeTask::FilterIteratorPair_t CTopology::getRuntimeTaskIteratorMatchingPath(
    const std::string& _pathPattern) const
{
    return m_topo->getRuntimeTaskIteratorMatchingPath(_pathPattern);
}

STopoRuntimeCollection::FilterIteratorPair_t CTopology::getRuntimeCollectionIteratorMatchingPath(
    const std::string& _pathPattern) const
{
    return m_topo->getRuntimeCollectionIteratorMatchingPath(_pathPattern);
}

size_t CTopology::getRequiredNofAgents() const
{
    return m_topo->getRequiredNofAgents();
}
