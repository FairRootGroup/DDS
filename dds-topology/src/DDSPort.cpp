// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSPort.h"
#include "DDSTopoElement.h"
// STD
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace boost::property_tree;
using namespace std;

DDSPort::DDSPort()
    : m_name("")
    , m_range(std::make_pair(10000, 50000))
{
}

DDSPort::~DDSPort()
{
}

void DDSPort::setName(const string& _name)
{
    m_name = _name;
}

void DDSPort::setRange(unsigned short _min, unsigned short _max)
{
    m_range = make_pair(_min, _max);
}

string DDSPort::getName() const
{
    return m_name;
}

const DDSPort::DDSPortRange_t& DDSPort::getRange() const
{
    return m_range;
}

void DDSPort::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& portPT = DDSTopoElement::findElement("port", _name, _pt);
        setName(portPT.get<string>("<xmlattr>.name"));
        setRange(portPT.get<unsigned int>("<xmlattr>.min"), portPT.get<unsigned int>("<xmlattr>.max"));
    }
    catch (ptree_error& error)
    {
        throw logic_error("Unable to initialize port " + _name + " error:" + error.what());
    }
}

string DDSPort::toString() const
{
    stringstream ss;
    ss << "DDSPort: m_name=" << m_name << " m_range=(" << m_range.first << ", " << m_range.second << ")";
    return ss.str();
}

ostream& operator<<(ostream& _strm, const DDSPort& _port)
{
    _strm << _port.toString();
    return _strm;
}
