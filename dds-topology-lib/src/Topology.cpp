// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Topology.h"
#include "TopologyParserXML.h"
#include "TopoIndex.h"
#include "TopoUtils.h"
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

void CTopology::init(const std::string& _fileName, bool _initForTest)
{
    /// FIXME Use parser based on the file extension.

    CTopologyParserXML parser;
    m_main = make_shared<CTaskGroup>();
    parser.parse(_fileName, m_main);

    m_topoIndexToTopoElementMap.clear();
    FillTopoIndexToTopoElementMap(m_main);

    m_counterMap.clear();
    m_hashToTaskMap.clear();
    m_hashToTaskCollectionMap.clear();
    m_currentTaskCollectionHashPath = "";
    FillHashToTopoElementMap(m_main, _initForTest);
}

const CTopology::TopoIndexToTopoElementMap_t CTopology::getTopoIndexToTopoElementMap() const
{
    return m_topoIndexToTopoElementMap;
}

const CTopology::HashToTaskMap_t CTopology::getHashToTaskMap() const
{
    return m_hashToTaskMap;
}

const CTopology::HashToTaskCollectionMap_t CTopology::getHashToTaskCollectionMap() const
{
    return m_hashToTaskCollectionMap;
}

const CTopology::HashPathToTaskMap_t& CTopology::getHashPathToTaskMap() const
{
    return m_hashPathToTaskMap;
}

const CTopology::HashPathToTaskCollectionMap_t& CTopology::getHashPathToTaskCollectionMap() const
{
    return m_hashPathToTaskCollectionMap;
}

TopoElementPtr_t CTopology::getTopoElementByTopoIndex(const CTopoIndex& _index) const
{
    auto it = m_topoIndexToTopoElementMap.find(_index);
    if (it == m_topoIndexToTopoElementMap.end())
        throw runtime_error("Can not find element with index " + _index.getPath());
    return it->second;
}

TaskPtr_t CTopology::getTaskByHash(uint64_t _hash) const
{
    auto it = m_hashToTaskMap.find(_hash);
    if (it == m_hashToTaskMap.end())
        throw runtime_error("Can not find element with hash " + to_string(_hash));
    return it->second;
}

TaskCollectionPtr_t CTopology::getTaskCollectionByHash(uint64_t _hash) const
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
        condition = [](HashToTaskMap_t::value_type) -> bool
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
        condition = [](HashToTaskCollectionMap_t::value_type) -> bool
        {
            return true;
        };
    }
    TaskCollectionIterator_t begin_iterator(condition, m_hashToTaskCollectionMap.begin(),
                                            m_hashToTaskCollectionMap.end());
    TaskCollectionIterator_t end_iterator(condition, m_hashToTaskCollectionMap.end(), m_hashToTaskCollectionMap.end());
    return make_pair(begin_iterator, end_iterator);
}

CTopology::TaskIteratorPair_t CTopology::getTaskIteratorForPropertyId(const std::string& _propertyId) const
{
    return getTaskIterator([&_propertyId](CTopology::TaskIterator_t::value_type value) -> bool
                           {
        TaskPtr_t task = value.second;
        const TopoPropertyPtrVector_t& properties = task->getProperties();
        for (const auto& v : properties)
        {
            if (v->getId() == _propertyId)
                return true;
        }
        return false;
    });
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

void CTopology::FillHashToTopoElementMap(const TopoElementPtr_t& _element, bool _fillHashPathMaps)
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

        if (_fillHashPathMaps)
        {
            m_hashPathToTaskMap[hashPath] = task;
        }

        uint64_t crc = dds::crc64(hashPath);
        if (m_hashToTaskMap.find(crc) != m_hashToTaskMap.end())
        {
            std::stringstream ss;
            ss << "The same hash detected <" << crc << "> for " << hashPath << std::endl;
            throw runtime_error(ss.str());
        }
        else
        {
            m_hashToTaskMap[crc] = task;
        }
        return;
    }
    else if (_element->getType() == ETopoType::COLLECTION)
    {
        TaskCollectionPtr_t collection = dynamic_pointer_cast<CTaskCollection>(_element);

        std::string path = collection->getPath();
        size_t counter = ++m_counterMap[path];
        m_currentTaskCollectionHashPath = path + "_" + to_string(counter);

        if (_fillHashPathMaps)
        {
            m_hashPathToTaskCollectionMap[m_currentTaskCollectionHashPath] = collection;
        }

        uint64_t crc = dds::crc64(m_currentTaskCollectionHashPath);
        if (m_hashToTaskCollectionMap.find(crc) != m_hashToTaskCollectionMap.end())
        {
            std::stringstream ss;
            ss << "The same hash detected <" << crc << "> for " << m_currentTaskCollectionHashPath << std::endl;
            throw runtime_error(ss.str());
        }
        else
        {
            m_hashToTaskCollectionMap[crc] = collection;
        }

        const auto& elements = collection->getElements();
        for (const auto& v : elements)
        {
            FillHashToTopoElementMap(v, _fillHashPathMaps);
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
                FillHashToTopoElementMap(v, _fillHashPathMaps);
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
