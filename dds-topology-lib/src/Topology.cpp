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
using namespace boost;

CTopology::CTopology()
    : m_main()
    , m_topoIndexToTopoElementMap()
    , m_hashToTaskMap()
    , m_hashToTaskCollectionMap()
    , m_counterMap()
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

    m_counterMap.clear();
    FillHashToTopoElementMap(m_main);
}

TopoElementPtr_t CTopology::getTopoElementByTopoIndex(const CTopoIndex& _index) const
{
    auto it = m_topoIndexToTopoElementMap.find(_index);
    if (it == m_topoIndexToTopoElementMap.end())
        throw runtime_error("Can not find element with index " + _index.getPath());
    return it->second;
}

TaskPtr_t CTopology::getTaskByHash(size_t _hash) const
{
    auto it = m_hashToTaskMap.find(_hash);
    if (it == m_hashToTaskMap.end())
        throw runtime_error("Can not find element with hash " + to_string(_hash));
    return it->second;
}

TaskCollectionPtr_t CTopology::getTaskCollectionByHash(size_t _hash) const
{
    auto it = m_hashToTaskCollectionMap.find(_hash);
    if (it == m_hashToTaskCollectionMap.end())
        throw runtime_error("Can not find element with hash " + to_string(_hash));
    return it->second;
}

CTopology::TaskIteratorPair_t CTopology::getTaskIterator(TaskCondition_t _condition) const
{
    TaskCondition_t condition = _condition;
    if (condition == nullptr)
    {
        condition = [](std::pair<size_t, TaskPtr_t>) -> bool
        {
            return true;
        };
    }
    TaskIterator_t begin_iterator(condition, m_hashToTaskMap.begin(), m_hashToTaskMap.end());
    TaskIterator_t end_iterator(condition, m_hashToTaskMap.end(), m_hashToTaskMap.end());
    return make_pair(begin_iterator, end_iterator);
}

CTopology::TaskCollectionIteratorPair_t CTopology::getTaskCollectionIterator(TaskCollectionCondition_t _condition) const
{
    TaskCollectionCondition_t condition = _condition;
    if (condition == nullptr)
    {
        condition = [](std::pair<size_t, TaskCollectionPtr_t>) -> bool
        {
            return true;
        };
    }
    TaskCollectionIterator_t begin_iterator(condition, m_hashToTaskCollectionMap.begin(),
                                            m_hashToTaskCollectionMap.end());
    TaskCollectionIterator_t end_iterator(condition, m_hashToTaskCollectionMap.end(), m_hashToTaskCollectionMap.end());
    return make_pair(begin_iterator, end_iterator);
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

void CTopology::FillHashToTopoElementMap(const TopoElementPtr_t& _element)
{
    if (_element->getType() == ETopoType::TASK)
    {
        TaskPtr_t task = dynamic_pointer_cast<CTask>(_element);
        std::string path;
        if (task->getParent()->getType() == ETopoType::COLLECTION)
        {
            path = m_currentTaskCollectionHashPath + "/" + task->getId();
        }
        else
        {
            path = _element->getPath();
        }

        size_t counter = ++m_counterMap[path];
        std::string hashPath = path + "_" + to_string(counter);
        m_hashPathToTaskMap[hashPath] = task;

        std::hash<string> hash;
        m_hashToTaskMap[hash(hashPath)] = task;
        return;
    }
    else if (_element->getType() == ETopoType::COLLECTION)
    {
        TaskCollectionPtr_t collection = dynamic_pointer_cast<CTaskCollection>(_element);

        std::string path = collection->getPath();
        size_t counter = ++m_counterMap[path];
        m_currentTaskCollectionHashPath = path + "_" + to_string(counter);
        m_hashPathToTaskCollectionMap[m_currentTaskCollectionHashPath] = collection;

        std::hash<std::string> hash;
        m_hashToTaskCollectionMap[hash(m_currentTaskCollectionHashPath)] = collection;

        const auto& elements = collection->getElements();
        for (const auto& v : elements)
        {
            FillHashToTopoElementMap(v);
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
                FillHashToTopoElementMap(v);
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
    ss << "  m_hashToTaskMap size=" << m_hashToTaskMap.size() << "\n";
    for (const auto& v : m_hashToTaskMap)
    {
        ss << "    " << v.first << " -> " << v.second->getPath() << "\n";
    }
    ss << "  m_hashToTaskCollectionMap size=" << m_hashToTaskCollectionMap.size() << "\n";
    for (const auto& v : m_hashToTaskCollectionMap)
    {
        ss << "    " << v.first << " -> " << v.second->getPath() << "\n";
    }
    ss << "  m_hashPathToTaskMap size=" << m_hashPathToTaskMap.size() << "\n";
    for (const auto& v : m_hashPathToTaskMap)
    {
        ss << "    " << v.first << " -> " << v.second->getPath() << "\n";
    }
    ss << "  m_hashPathToTaskCollectionMap size=" << m_hashPathToTaskCollectionMap.size() << "\n";
    for (const auto& v : m_hashPathToTaskCollectionMap)
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
