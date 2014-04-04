// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopology.h"
#include "DDSTopologyParserXML.h"
#include "DDSIndex.h"
// STD
#include <string>

using namespace std;

DDSTopology::DDSTopology() : m_main()
{
}

DDSTopology::~DDSTopology()
{
}

DDSTaskGroupPtr_t DDSTopology::getMainGroup() const
{
    return m_main;
}

void DDSTopology::init(const std::string& _fileName)
{
    /// FIXME Use parser based on the file extension.

    DDSTopologyParserXML parser;
    m_main = make_shared<DDSTaskGroup>();
    parser.parse(_fileName, m_main);
    FillIndexToTopoElementMap(m_main);
}

DDSTopoElementPtr_t DDSTopology::getTopoElementByIndex(const DDSIndex& _index) const
{
    auto it = m_indexToTopoElementMap.find(_index);
    if (it == m_indexToTopoElementMap.end())
        throw runtime_error("Can not find element with index " + _index.getPath());
    return it->second;
}

void DDSTopology::FillIndexToTopoElementMap(const DDSTopoElementPtr_t& _element)
{
    if (_element->getType() == DDSTopoType::TASK)
    {
        m_indexToTopoElementMap[_element->getPath()] = _element;
        return;
    }
    DDSTaskContainerPtr_t container = dynamic_pointer_cast<DDSTaskContainer>(_element);
    const auto& elements = container->getElements();
    for (const auto& v : elements)
    {
        m_indexToTopoElementMap[v->getPath()] = v;
        FillIndexToTopoElementMap(v);
    }
}

string DDSTopology::toString() const
{
    stringstream ss;

    return ss.str();
}

ostream& operator<<(ostream& _strm, const DDSTopology& _topology)
{
    _strm << _topology.toString();
    return _strm;
}
