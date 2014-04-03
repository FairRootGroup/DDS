// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopology.h"
#include "DDSTopologyParserXML.h"
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
    FillPathToTopoElementMap(m_main);
}

DDSTopoElementPtr_t DDSTopology::getTopoElementByPath(const string& _path) const
{
    auto it = m_pathToTopoElementMap.find(_path);
    if (it == m_pathToTopoElementMap.end())
        throw runtime_error("Can not find element with path " + _path);
    return it->second;
}

void DDSTopology::FillPathToTopoElementMap(const DDSTopoElementPtr_t& _element)
{
    if (_element->getType() == DDSTopoType::TASK)
    {
        m_pathToTopoElementMap[_element->getPath()] = _element;
        return;
    }
    DDSTaskContainerPtr_t container = dynamic_pointer_cast<DDSTaskContainer>(_element);
    const auto& elements = container->getElements();
    for (const auto& v : elements)
    {
        m_pathToTopoElementMap[v->getPath()] = v;
        FillPathToTopoElementMap(v);
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
