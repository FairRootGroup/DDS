// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "TopoVars.h"
#include "TopoUtils.h"
// STD
#include <iostream>
//#include <regex>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/regex.hpp>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;
namespace fs = boost::filesystem;

CTopoVars::CTopoVars()
    : CTopoBase("")
{
    setType(CTopoBase::EType::TOPO_VARS);
}

CTopoVars::CTopoVars(const std::string& _name)
    : CTopoBase(_name)
{
    setType(CTopoBase::EType::TOPO_VARS);
}

CTopoVars::~CTopoVars()
{
}

void CTopoVars::initFromXML(const string& _filepath)
{
    fs::path pathXML = { _filepath };
    if (!fs::exists(pathXML))
        throw runtime_error("Can't locate the given topo file: " + pathXML.string());

    ifstream stream(pathXML.string());
    if (stream.is_open())
    {
        m_pPropTreePtr = make_unique<propTreePtr_t::element_type>();
        read_xml(stream, *m_pPropTreePtr, xml_parser::no_comments);
        stream.seekg(0);

        initFromPropertyTree(*m_pPropTreePtr);
    }
    else
    {
        throw runtime_error("Can't open the given topo file: " + pathXML.string());
    }
}

void CTopoVars::saveToXML(const string& _filepath)
{
    // The vars object can't be saved as is. It must have been initilized from a topology file first.
    if (!m_pPropTreePtr)
        runtime_error("The CTopoVars object can't be saved to XML. It wasn't iniutlized from a topology file.");

    ofstream stream(_filepath);
    if (stream.is_open())
    {
        saveToPropertyTree(*m_pPropTreePtr);
        auto settings = xml_writer_settings<typename ptree::key_type>(' ', 4);
        write_xml(stream, *m_pPropTreePtr, settings);
        m_pPropTreePtr.reset();
    }
    else
    {
        throw runtime_error("Can't create topo file: " + _filepath);
    }
}

const CTopoVars::varMap_t& CTopoVars::getMap() const
{
    return m_map;
}

void CTopoVars::initFromPropertyTree(const ptree& _pt)
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

void CTopoVars::saveToPropertyTree(ptree& _pt)
{
    try
    {
        // Remove exiting vars, if any
        ptree& pt = _pt.get_child("topology");
        for (auto it = pt.begin(); it != pt.end(); ++it)
        {
            if (it->first == TopoTypeToDeclTag(CTopoBase::EType::TOPO_VARS))
            {
                pt.erase(it);
            }
        }

        // pupulate the topoloogy with new vars
        ptree& topo = _pt.get_child("topology");
        for (const auto& v : m_map)
        {
            ptree ptVar;
            ptVar.put("<xmlattr>.name", v.first);
            ptVar.put("<xmlattr>.value", v.second);
            // make sure their are on top of the xml
            topo.push_front({ "var", ptVar });
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

bool CTopoVars::add(const varName_t& _name, const varValue_t& _value)
{
    if (_name.empty())
        return false;

    const auto retVal = m_map.insert(make_pair(_name, _value));
    return retVal.second;
}

bool CTopoVars::update(const varName_t& _name, const varValue_t& _newValue)
{
    auto found = m_map.find(_name);
    if (found == m_map.end())
        return false;

    found->second = _newValue;
    return true;
}
