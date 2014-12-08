// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Requirement.h"
// STD
#include <iostream>
//#include <regex>
#include <boost/regex.hpp>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

CRequirement::CRequirement()
    : CTopoBase()
    , m_hostPattern()
{
    setType(ETopoType::REQUIREMENT);
}

CRequirement::~CRequirement()
{
}

const std::string& CRequirement::getHostPattern() const
{
    return m_hostPattern;
}

void CRequirement::setHostPattern(const std::string& _hostPattern)
{
    m_hostPattern = _hostPattern;
}

bool CRequirement::hostPatterMatches(const std::string& _host) const
{
    if (getHostPattern().empty())
        return true;
    const boost::regex e(getHostPattern());
    return boost::regex_match(_host, e);
}

void CRequirement::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& requirementPT = CTopoBase::findElement(ETopoType::REQUIREMENT, _name, _pt.get_child("topology"));
        setId(requirementPT.get<string>("<xmlattr>.id"));
        setHostPattern(requirementPT.get<std::string>("hostPattern.<xmlattr>.value", ""));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize property " + _name + " error:" + error.what());
    }
}

string CRequirement::toString() const
{
    stringstream ss;
    ss << "DDSRequirement: m_id=" << getId() << " m_hostPattern=" << getHostPattern();
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CRequirement& _requirement)
{
    _strm << _requirement.toString();
    return _strm;
}
