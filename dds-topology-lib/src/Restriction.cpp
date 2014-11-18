// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Restriction.h"
// STD
#include <iostream>
#include <regex>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

CRestriction::CRestriction()
    : CTopoBase()
    , m_hostPattern()
    , m_userPattern()
{
    setType(ETopoType::RESTRICTION);
}

CRestriction::~CRestriction()
{
}

const std::string& CRestriction::getHostPattern() const
{
    return m_hostPattern;
}

const std::string& CRestriction::getUserPattern() const
{
    return m_userPattern;
}

void CRestriction::setHostPattern(const std::string& _hostPattern)
{
    m_hostPattern = _hostPattern;
}

void CRestriction::setUserPattern(const std::string& _userPattern)
{
    m_userPattern = _userPattern;
}

bool CRestriction::hostPatterMatches(const std::string& _host) const
{
    if (getHostPattern().empty())
        return true;
    const std::regex e(getHostPattern());
    return std::regex_match(_host, e);
}

bool CRestriction::userPatterMatches(const std::string& _user) const
{
    if (getUserPattern().empty())
        return true;
    const std::regex e(getUserPattern());
    return std::regex_match(_user, e);
}

void CRestriction::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& restrictionPT = CTopoBase::findElement(ETopoType::RESTRICTION, _name, _pt.get_child("topology"));
        setId(restrictionPT.get<string>("<xmlattr>.id"));
        setHostPattern(restrictionPT.get<std::string>("hostPattern", ""));
        setUserPattern(restrictionPT.get<std::string>("userPattern", ""));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize property " + _name + " error:" + error.what());
    }
}

string CRestriction::toString() const
{
    stringstream ss;
    ss << "DDSRestriction: m_id=" << getId() << " m_hostPattern=" << getHostPattern()
       << " m_userPattern=" << getUserPattern();
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CRestriction& _restriction)
{
    _strm << _restriction.toString();
    return _strm;
}
