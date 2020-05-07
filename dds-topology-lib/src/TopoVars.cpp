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
using namespace topology_api;

CTopoVars::CTopoVars(const std::string& _name)
    : CTopoBase(_name)
{
    setType(CTopoBase::EType::TOPO_VARS);
}

CTopoVars::~CTopoVars()
{
}

const CTopoVars::varMap_t& CTopoVars::getMap() const
{
    return m_map;
}

void CTopoVars::initFromPropertyTree(const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& pt = _pt.get_child("topology");

        for (const auto& v : pt)
        {
            const auto& elementPT = v.second;
            if (v.first == TopoTypeToDeclTag(CTopoBase::EType::TOPO_VARS))
            {
                m_map[elementPT.get<string>("<xmlattr>.name")] = elementPT.get<string>("<xmlattr>.value");
            }
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw runtime_error("Unable to initialize task collection " + getName() + " error: " + error.what());
    }
}

void CTopoVars::saveToPropertyTree(boost::property_tree::ptree& _pt)
{
    try
    {
        int counter = 0;
        for (const auto& v : m_map)
        {
            std::string tag("topology.var.<xmlattr>");
            if (counter == 0)
            {
                _pt.put(tag + ".name", v.first);
                _pt.put(tag + ".value", v.second);
            }
            else
            {
                _pt.add(tag + ".name", v.first);
                _pt.add(tag + ".value", v.second);
            }

            counter++;
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw runtime_error("Unable to initialize topo vars " + getName() + " error: " + error.what());
    }
}

string CTopoVars::toString() const
{
    stringstream ss;
    ss << "DDSTopoVars: m_name=" << getName() << endl;
    for (const auto& v : m_map)
    {
        ss << v.first << " --> " << v.second << endl;
    }

    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoVars& _vars)
{
    _strm << _vars.toString();
    return _strm;
}

string CTopoVars::hashString() const
{
    stringstream ss;
    ss << "|Vars|" << getName() << "|";
    for (const auto& var : getMap())
    {
        ss << var.first << "|" << var.second << "|";
    }
    return ss.str();
}
