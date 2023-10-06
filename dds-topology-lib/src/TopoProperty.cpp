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

CTopoProperty::CTopoProperty(const std::string& _name)
    : CTopoBase(_name)
{
    setType(CTopoBase::EType::TOPO_PROPERTY);
}

CTopoProperty::~CTopoProperty()
{
}

const std::string& CTopoProperty::getValue() const
{
    return m_value;
}

CTopoProperty::EAccessType CTopoProperty::getAccessType() const
{
    return m_accessType;
}

void CTopoProperty::setValue(const std::string& _value)
{
    m_value = _value;
}

void CTopoProperty::setAccessType(CTopoProperty::EAccessType _accessType)
{
    m_accessType = _accessType;
}

CTopoProperty::EScopeType CTopoProperty::getScopeType() const
{
    return m_scopeType;
}

void CTopoProperty::setScopeType(CTopoProperty::EScopeType _scopeType)
{
    m_scopeType = _scopeType;
}

void CTopoProperty::initFromPropertyTree(const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& propertyPT =
            (FindElementInPropertyTree(CTopoBase::EType::TOPO_PROPERTY, getName(), _pt.get_child("topology")))->second;

        boost::optional<const ptree&> childScope = propertyPT.get_child_optional("<xmlattr>.scope");
        if (childScope)
        {
            setScopeType(TagToPropertyScopeType(propertyPT.get<string>("<xmlattr>.scope")));
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize property " + getName() + " error:" + error.what());
    }
}

void CTopoProperty::saveToPropertyTree(boost::property_tree::ptree& _pt)
{
    try
    {
        std::string tag("topology.property.<xmlattr>");
        _pt.put(tag + ".name", getName());
        _pt.put(tag + ".scope", PropertyScopeTypeToTag(getScopeType()));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to save property " + getName() + " error:" + error.what());
    }
}

string CTopoProperty::toString() const
{
    stringstream ss;
    ss << "DDSTopoProperty: m_name=" << getName() << " m_value=" << getValue();
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoProperty& _property)
{
    _strm << _property.toString();
    return _strm;
}

string CTopoProperty::hashString() const
{
    stringstream ss;
    ss << "|Property|" << getName() << "|" << getValue() << "|" << PropertyAccessTypeToTag(getAccessType()) << "|"
       << PropertyScopeTypeToTag(getScopeType()) << "|";
    return ss.str();
}
