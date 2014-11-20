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
}

TopoElementPtr_t CTopology::getTopoElementByTopoIndex(const CTopoIndex& _index) const
{
    auto it = m_topoIndexToTopoElementMap.find(_index);
    if (it == m_topoIndexToTopoElementMap.end())
        throw runtime_error("Can not find element with index " + _index.getPath());
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
