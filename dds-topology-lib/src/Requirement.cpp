// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Requirement.h"
// STD
#include <iostream>
#include <regex>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

CRequirement::CRequirement()
    : CTopoBase()
    , m_hostPattern()
    , m_userPattern()
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

const std::string& CRequirement::getUserPattern() const
{
    return m_userPattern;
}

void CRequirement::setHostPattern(const std::string& _hostPattern)
{
    m_hostPattern = _hostPattern;
}

void CRequirement::setUserPattern(const std::string& _userPattern)
{
    m_userPattern = _userPattern;
}

bool CRequirement::hostPatterMatches(const std::string& _host) const
{
    if (getHostPattern().empty())
        return true;
    const std::regex e(getHostPattern());
    return std::regex_match(_host, e);
}

bool CRequirement::userPatterMatches(const std::string& _user) const
{
    if (getUserPattern().empty())
        return true;
    const std::regex e(getUserPattern());
    return std::regex_match(_user, e);
}

void CRequirement::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& requirementPT = CTopoBase::findElement(ETopoType::REQUIREMENT, _name, _pt.get_child("topology"));
        setId(requirementPT.get<string>("<xmlattr>.id"));
        setHostPattern(requirementPT.get<std::string>("hostPattern", ""));
        setUserPattern(requirementPT.get<std::string>("userPattern", ""));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize property " + _name + " error:" + error.what());
    }
}

string CRequirement::toString() const
{
    stringstream ss;
    ss << "DDSRequirement: m_id=" << getId() << " m_hostPattern=" << getHostPattern()
       << " m_userPattern=" << getUserPattern();
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CRequirement& _requirement)
{
    _strm << _requirement.toString();
    return _strm;
}
