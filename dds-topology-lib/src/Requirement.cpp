// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Requirement.h"
#include "TopoUtils.h"
// STD
#include <boost/regex.hpp>
#include <iostream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CRequirement::CRequirement()
    : CTopoBase()
    , m_value()
    , m_requirementType(ERequirementType::HostName)
{
    setType(ETopoType::REQUIREMENT);
}

CRequirement::~CRequirement()
{
}

const std::string& CRequirement::getValue() const
{
    return m_value;
}

ERequirementType CRequirement::getRequirementType() const
{
    return m_requirementType;
}

void CRequirement::setValue(const std::string& _value)
{
    m_value = _value;
}

void CRequirement::setRequirementType(ERequirementType _requirementType)
{
    m_requirementType = _requirementType;
}

void CRequirement::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& requirementPT = CTopoBase::findElement(ETopoType::REQUIREMENT, _name, _pt.get_child("topology"));
        setId(requirementPT.get<string>("<xmlattr>.id"));
        setValue(requirementPT.get<std::string>("<xmlattr>.value", ""));
        setRequirementType(TagToRequirementType(requirementPT.get<std::string>("<xmlattr>.type", "")));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize property " + _name + " error:" + error.what());
    }
}

string CRequirement::toString() const
{
    stringstream ss;
    ss << "DDSRequirement: m_id=" << getId() << " m_value=" << getValue();
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CRequirement& _requirement)
{
    _strm << _requirement.toString();
    return _strm;
}
