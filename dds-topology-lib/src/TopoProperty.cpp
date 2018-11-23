// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoProperty.h"
#include "TopoUtils.h"
// STD
#include <iostream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTopoProperty::CTopoProperty()
    : CTopoBase()
    , m_value()
    , m_accessType(EPropertyAccessType::READWRITE)
    , m_scopeType(EPropertyScopeType::GLOBAL)
{
    setType(ETopoType::TOPO_PROPERTY);
}

CTopoProperty::~CTopoProperty()
{
}

const std::string& CTopoProperty::getValue() const
{
    return m_value;
}

EPropertyAccessType CTopoProperty::getAccessType() const
{
    return m_accessType;
}

void CTopoProperty::setValue(const std::string& _value)
{
    m_value = _value;
}

void CTopoProperty::setAccessType(EPropertyAccessType _accessType)
{
    m_accessType = _accessType;
}

EPropertyScopeType CTopoProperty::getScopeType() const
{
    return m_scopeType;
}

void CTopoProperty::setScopeType(EPropertyScopeType _scopeType)
{
    m_scopeType = _scopeType;
}

void CTopoProperty::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& propertyPT = CTopoBase::findElement(ETopoType::TOPO_PROPERTY, _name, _pt.get_child("topology"));
        setId(propertyPT.get<string>("<xmlattr>.id"));
        setScopeType(TagToPropertyScopeType(propertyPT.get<string>("<xmlattr>.scope")));
        // setAccessType(TagToPropertyAccessType(propertyPT.get<string>("<xmlattr>.access")));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize property " + _name + " error:" + error.what());
    }
}

string CTopoProperty::toString() const
{
    stringstream ss;
    ss << "DDSTopoProperty: m_id=" << getId() << " m_value=" << getValue();
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoProperty& _property)
{
    _strm << _property.toString();
    return _strm;
}
