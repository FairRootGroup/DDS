// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoProperty.h"
// STD
#include <iostream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

CTopoProperty::CTopoProperty()
    : CTopoBase()
    , m_value()
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

void CTopoProperty::setValue(const std::string& _value)
{
    m_value = _value;
}

void CTopoProperty::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& propertyPT = CTopoBase::findElement(ETopoType::TOPO_PROPERTY, _name, _pt.get_child("topology"));
        setName(propertyPT.get<string>("<xmlattr>.name"));
        setValue(propertyPT.get<string>("<xmlattr>.default", ""));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize port " + _name + " error:" + error.what());
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
