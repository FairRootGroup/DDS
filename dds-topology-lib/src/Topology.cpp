// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Topology.h"
#include "TopologyParserXML.h"
#include "Index.h"
// STD
#include <string>

using namespace std;
using namespace dds;

CTopology::CTopology()
    : m_main()
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
    FillIndexToTopoElementMap(m_main);
}

TopoElementPtr_t CTopology::getTopoElementByIndex(const CIndex& _index) const
{
    auto it = m_indexToTopoElementMap.find(_index);
    if (it == m_indexToTopoElementMap.end())
        throw runtime_error("Can not find element with index " + _index.getPath());
    return it->second;
}

void CTopology::FillIndexToTopoElementMap(const TopoElementPtr_t& _element)
{
    if (_element->getType() == ETopoType::TASK)
    {
        m_indexToTopoElementMap[_element->getPath()] = _element;
        return;
    }
    TaskContainerPtr_t container = dynamic_pointer_cast<CTaskContainer>(_element);
    const auto& elements = container->getElements();
    for (const auto& v : elements)
    {
        m_indexToTopoElementMap[v->getPath()] = v;
        FillIndexToTopoElementMap(v);
    }
}

string CTopology::toString() const
{
    stringstream ss;

    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopology& _topology)
{
    _strm << _topology.toString();
    return _strm;
}
