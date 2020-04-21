// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoRequirement.h"
#include "TopoUtils.h"
// STD
#include <boost/regex.hpp>
#include <iostream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTopoRequirement::CTopoRequirement(const std::string& _name)
    : CTopoBase(_name)
{
    setType(CTopoBase::EType::REQUIREMENT);
}

CTopoRequirement::~CTopoRequirement()
{
}

const std::string& CTopoRequirement::getValue() const
{
    return m_value;
}

CTopoRequirement::EType CTopoRequirement::getRequirementType() const
{
    return m_requirementType;
}

void CTopoRequirement::setValue(const std::string& _value)
{
    m_value = _value;
}

void CTopoRequirement::setRequirementType(CTopoRequirement::EType _requirementType)
{
    m_requirementType = _requirementType;
}

void CTopoRequirement::initFromPropertyTree(const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& requirementPT =
            FindElementInPropertyTree(CTopoBase::EType::REQUIREMENT, getName(), _pt.get_child("topology"));
        setValue(requirementPT.get<std::string>("<xmlattr>.value", ""));
        setRequirementType(TagToRequirementType(requirementPT.get<std::string>("<xmlattr>.type", "")));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize property " + getName() + " error:" + error.what());
    }
}

void CTopoRequirement::saveToPropertyTree(boost::property_tree::ptree& _pt)
{
    try
    {
        std::string tag("topology.declrequirement.<xmlattr>");
        _pt.put(tag + ".name", getName());
        _pt.put(tag + ".value", getValue());
        _pt.put(tag + ".type", RequirementTypeToTag(getRequirementType()));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to save requirement " + getName() + " error:" + error.what());
    }
}

string CTopoRequirement::toString() const
{
    stringstream ss;
    ss << "DDSRequirement: name=" << getName() << " type=" << RequirementTypeToTag(getRequirementType())
       << " value=" << getValue();
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoRequirement& _requirement)
{
    _strm << _requirement.toString();
    return _strm;
}
