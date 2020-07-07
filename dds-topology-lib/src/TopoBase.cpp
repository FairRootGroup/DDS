// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoBase.h"
#include "TopoParserXML.h"
// STD
#include <iostream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTopoBase::CTopoBase(const std::string& _name)
    : m_name(_name)
{
}

CTopoBase::~CTopoBase()
{
}

void CTopoBase::setType(CTopoBase::EType _type)
{
    m_type = _type;
}

void CTopoBase::setName(const string& _name)
{
    m_name = _name;
}

void CTopoBase::setParent(CTopoBase* _parent)
{
    m_parent = _parent;
}

const string& CTopoBase::getName() const
{
    return m_name;
}

CTopoBase::EType CTopoBase::getType() const
{
    return m_type;
}

CTopoBase* CTopoBase::getParent() const
{
    return m_parent;
}

string CTopoBase::getPath() const
{
    if (getParent() == nullptr)
    {
        return getName();
    }
    else
    {
        return getParent()->getPath() + "/" + getName();
    }
}

void CTopoBase::initFromXML(const std::string& _filepath,
                            const std::string& _schemaFilepath,
                            std::string* _topologyName)
{
    boost::property_tree::ptree pt;
    CTopoParserXML::parse(pt, _filepath, _schemaFilepath, _topologyName);
    this->initFromPropertyTree(pt);
}

void CTopoBase::initFromXML(std::istream& _stream, const std::string& _schemaFilepath, std::string* _topologyName)
{
    boost::property_tree::ptree pt;
    CTopoParserXML::parse(pt, _stream, _schemaFilepath, _topologyName);
    this->initFromPropertyTree(pt);
}

string CTopoBase::toString() const
{
    stringstream ss;
    ss << "TopoBase: m_name=" << m_name;
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoBase& _element)
{
    _strm << _element.toString();
    return _strm;
}
