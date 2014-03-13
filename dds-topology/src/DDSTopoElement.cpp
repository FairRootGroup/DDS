// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopoElement.h"
// STD
#include <iostream>

using namespace std;
using namespace boost::property_tree;

DDSTopoElement::DDSTopoElement()
    : m_name("")
    , m_type(DDSTopoElementType::NONE)
{
}

DDSTopoElement::~DDSTopoElement()
{
}

void DDSTopoElement::setType(DDSTopoElementType _type)
{
    m_type = _type;
}

void DDSTopoElement::setName(const string& _name)
{
    m_name = _name;
}

string DDSTopoElement::getName() const
{
    return m_name;
}

DDSTopoElementType DDSTopoElement::getType() const
{
    return m_type;
}

const ptree& DDSTopoElement::findElement(const string& _tag, const string& _name, const ptree& _pt)
{
    // Acceptable tags
    if (_tag != "port" && _tag != "task" && _tag != "collection" && _tag != "group" && _tag != "main")
        throw logic_error("Unknown tag " + _tag);

    if (_tag == "main")
        return _pt.get_child("topology.main");

    const ptree& topPT = (_tag != "group") ? _pt.get_child("topology") : _pt.get_child("topology.main");
    for (const auto& v : topPT)
    {
        if (v.first == _tag)
        {
            const auto& elementPT = v.second;
            if (elementPT.get<string>("<xmlattr>.name") == _name)
            {
                return elementPT;
            }
        }
    }

    throw logic_error("Element not found in property tree " + _name);
}

string DDSTopoElement::toString() const
{
    std::stringstream ss;
    ss << "DDSTopoElement: m_name=" << m_name;
    return ss.str();
}

std::ostream& operator<<(std::ostream& _strm, const DDSTopoElement& _element)
{
    _strm << _element.toString();
    return _strm;
}
