// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoBase.h"
#include "TopoUtils.h"
// STD
#include <iostream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

CTopoBase::CTopoBase()
    : m_name("")
    , m_type(ETopoType::TOPO_BASE)
    , m_parent(nullptr)
{
}

CTopoBase::~CTopoBase()
{
}

void CTopoBase::setType(ETopoType _type)
{
    m_type = _type;
}

void CTopoBase::setName(const string& _name)
{
    m_name = _name;
}

void CTopoBase::setParent(CTopoBase* _parent)
{
    m_parent = _parent;
}

string CTopoBase::getName() const
{
    return m_name;
}

ETopoType CTopoBase::getType() const
{
    return m_type;
}

CTopoBase* CTopoBase::getParent() const
{
    return m_parent;
}

string CTopoBase::getPath() const
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

CIndex CTopoBase::getIndex() const
{
    return CIndex(getPath());
}

const ptree& CTopoBase::findElement(ETopoType _type, const string& _name, const ptree& _pt)
{
    const ptree* result = nullptr;
    for (const auto& v : _pt)
    {
        const auto& elementPT = v.second;
        if (v.first == TopoTypeToTag(_type) && elementPT.get<string>("<xmlattr>.name") == _name)
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

string CTopoBase::toString() const
{
    stringstream ss;
    ss << "TopoBase: m_name=" << m_name;
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoBase& _element)
{
    _strm << _element.toString();
    return _strm;
}
