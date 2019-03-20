// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoBase.h"
// STD
#include <iostream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTopoBase::CTopoBase()
    : m_name("")
    , m_type(CTopoBase::EType::TOPO_BASE)
    , m_parent(nullptr)
{
}

CTopoBase::~CTopoBase()
{
}

void CTopoBase::setType(CTopoBase::EType _type)
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

CTopoBase::EType CTopoBase::getType() const
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
