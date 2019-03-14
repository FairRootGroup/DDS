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

CTopoRequirement::CTopoRequirement()
    : CTopoBase()
    , m_value()
    , m_requirementType(CTopoRequirement::EType::HostName)
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

void CTopoRequirement::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& requirementPT =
            CTopoBase::findElement(CTopoBase::EType::REQUIREMENT, _name, _pt.get_child("topology"));
        setId(requirementPT.get<string>("<xmlattr>.id"));
        setValue(requirementPT.get<std::string>("<xmlattr>.value", ""));
        setRequirementType(TagToRequirementType(requirementPT.get<std::string>("<xmlattr>.type", "")));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize property " + _name + " error:" + error.what());
    }
}

string CTopoRequirement::toString() const
{
    stringstream ss;
    ss << "DDSRequirement: id=" << getId() << " type=" << RequirementTypeToTag(getRequirementType())
       << " value=" << getValue();
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoRequirement& _requirement)
{
    _strm << _requirement.toString();
    return _strm;
}
