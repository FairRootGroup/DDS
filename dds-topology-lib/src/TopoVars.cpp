// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "TopoVars.h"
#include "TopoUtils.h"
// STD
#include <iostream>
//#include <regex>
#include <boost/regex.hpp>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

CTopoVars::CTopoVars()
: CTopoBase()
, m_map()
{
    setType(ETopoType::TOPO_VARS);
}

CTopoVars::~CTopoVars()
{
}

const CTopoVars::varMap_t& CTopoVars::getMap() const
{
    return m_map;
}

void CTopoVars::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& pt = _pt.get_child("topology");
        
        for (const auto& v : pt)
        {
            const auto& elementPT = v.second;
            if (v.first == TopoTypeToDeclTag(ETopoType::TOPO_VARS))
            {
                m_map[elementPT.get<string>("<xmlattr>.id")] = elementPT.get<string>("<xmlattr>.value");
            }
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw runtime_error("Unable to initialize task collection " + _name + " error: " + error.what());
    }
}

string CTopoVars::toString() const
{
    stringstream ss;
    ss << "DDSTopoVars: m_id=" << getId() << endl;
    for (const auto& v : m_map) {
        ss << v.first << " --> " << v.second << endl;
    }
    
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoVars& _vars)
{
    _strm << _vars.toString();
    return _strm;
}