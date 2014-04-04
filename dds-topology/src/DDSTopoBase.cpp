// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopoBase.h"
#include "DDSTopoUtils.h"
// STD
#include <iostream>

using namespace std;
using namespace boost::property_tree;

DDSTopoBase::DDSTopoBase()
    : m_name("")
    , m_type(DDSTopoType::TOPO_BASE)
    , m_parent(nullptr)
{
}

DDSTopoBase::~DDSTopoBase()
{
}

void DDSTopoBase::setType(DDSTopoType _type)
{
    m_type = _type;
}

void DDSTopoBase::setName(const string& _name)
{
    m_name = _name;
}

void DDSTopoBase::setParent(DDSTopoBase* _parent)
{
    m_parent = _parent;
}

string DDSTopoBase::getName() const
{
    return m_name;
}

DDSTopoType DDSTopoBase::getType() const
{
    return m_type;
}

DDSTopoBase* DDSTopoBase::getParent() const
{
    return m_parent;
}

string DDSTopoBase::getPath() const
{
    if (getParent() == nullptr)
    {
        return getName();
    }
    else
    {
        return getParent()->getPath() + "/" + getName();
    }
}

DDSIndex DDSTopoBase::getIndex() const
{
    return DDSIndex(getPath());
}

const ptree& DDSTopoBase::findElement(DDSTopoType _type, const string& _name, const ptree& _pt)
{
    const ptree* result = nullptr;
    for (const auto& v : _pt)
    {
        const auto& elementPT = v.second;
        if (v.first == DDSTopoTypeToTag(_type) && elementPT.get<string>("<xmlattr>.name") == _name)
        {
            if (result != nullptr)
                throw logic_error("Element \"" + _name + "\" has dublicated name.");
            result = &elementPT;
        }
    }
    if (result == nullptr)
        throw logic_error("Element \"" + _name + "\"not found in property tree.");

    return *result;
}

string DDSTopoBase::toString() const
{
    stringstream ss;
    ss << "DDSTopoBase: m_name=" << m_name;
    return ss.str();
}

ostream& operator<<(ostream& _strm, const DDSTopoBase& _element)
{
    _strm << _element.toString();
    return _strm;
}
