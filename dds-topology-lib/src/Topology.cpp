// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Topology.h"
#include "TopologyParserXML.h"
#include "TopoIndex.h"
// STD
#include <string>

using namespace std;
using namespace dds;

CTopology::CTopology()
    : m_main()
    , m_topoIndexToTopoElementMap()
    , m_indexToTaskMap()
    , m_indexToTaskCollectionMap()
    , m_taskCounter(0)
    , m_taskCollectionCounter(0)
{
}

CTopology::~CTopology()
{
}

TaskGroupPtr_t CTopology::getMainGroup() const
{
    return m_main;
}

void CTopology::init(const std::string& _fileName)
{
    /// FIXME Use parser based on the file extension.

    CTopologyParserXML parser;
    m_main = make_shared<CTaskGroup>();
    parser.parse(_fileName, m_main);

    FillTopoIndexToTopoElementMap(m_main);

    m_taskCounter = 0;
    m_taskCollectionCounter = 0;
    FillIndexToTopoElementMap(m_main);
}

TopoElementPtr_t CTopology::getTopoElementByTopoIndex(const CTopoIndex& _index) const
{
    auto it = m_topoIndexToTopoElementMap.find(_index);
    if (it == m_topoIndexToTopoElementMap.end())
        throw runtime_error("Can not find element with index " + _index.getPath());
    return it->second;
}

TaskPtr_t CTopology::getTaskByIndex(size_t _index) const
{
    auto it = m_indexToTaskMap.find(_index);
    if (it == m_indexToTaskMap.end())
        throw runtime_error("Can not find element with index " + to_string(_index));
    return it->second;
}

TaskCollectionPtr_t CTopology::getTaskCollectionByIndex(size_t _index) const
{
    auto it = m_indexToTaskCollectionMap.find(_index);
    if (it == m_indexToTaskCollectionMap.end())
        throw runtime_error("Can not find element with index " + to_string(_index));
    return it->second;
}

void CTopology::FillTopoIndexToTopoElementMap(const TopoElementPtr_t& _element)
{
    if (_element->getType() == ETopoType::TASK)
    {
        m_topoIndexToTopoElementMap[_element->getPath()] = _element;
        return;
    }
    TaskContainerPtr_t container = dynamic_pointer_cast<CTaskContainer>(_element);
    const auto& elements = container->getElements();
    for (const auto& v : elements)
    {
        m_topoIndexToTopoElementMap[v->getPath()] = v;
        FillTopoIndexToTopoElementMap(v);
    }
}

void CTopology::FillIndexToTopoElementMap(const TopoElementPtr_t& _element)
{
    if (_element->getType() == ETopoType::TASK)
    {
        m_indexToTaskMap[m_taskCounter++] = dynamic_pointer_cast<CTask>(_element);
        return;
    }
    else if (_element->getType() == ETopoType::COLLECTION)
    {
        TaskCollectionPtr_t collection = dynamic_pointer_cast<CTaskCollection>(_element);
        m_indexToTaskCollectionMap[m_taskCollectionCounter++] = collection;
        const auto& elements = collection->getElements();
        for (const auto& v : elements)
        {
            FillIndexToTopoElementMap(v);
        }
    }
    else if (_element->getType() == ETopoType::GROUP)
    {
        TaskGroupPtr_t group = dynamic_pointer_cast<CTaskGroup>(_element);
        const auto& elements = group->getElements();
        size_t n = group->getN();
        for (size_t i = 0; i < n; i++)
        {
            for (const auto& v : elements)
            {
                FillIndexToTopoElementMap(v);
            }
        }
    }
}

string CTopology::toString() const
{
    stringstream ss;
    ss << "CTopology:\n";
    ss << "  m_topoIndexToTopoElementMap size=" << m_topoIndexToTopoElementMap.size() << "\n";
    for (const auto& v : m_topoIndexToTopoElementMap)
    {
        ss << "    " << v.first.getPath() << " -> " << v.second->getPath() << "\n";
    }
    ss << "  m_indexToTaskMap size=" << m_indexToTaskMap.size() << "\n";
    for (const auto& v : m_indexToTaskMap)
    {
        ss << "    " << v.first << " -> " << v.second->getPath() << "\n";
    }
    ss << "  m_indexToTaskCollectionMap size=" << m_indexToTaskCollectionMap.size() << "\n";
    for (const auto& v : m_indexToTaskCollectionMap)
    {
        ss << "    " << v.first << " -> " << v.second->getPath() << "\n";
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopology& _topology)
{
    _strm << _topology.toString();
    return _strm;
}
