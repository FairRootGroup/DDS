// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Port.h"
#include "TopoElement.h"
#include "TopoUtils.h"
// STD
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace boost::property_tree;
using namespace std;
using namespace dds;

CPort::CPort()
    : CTopoProperty()
    , m_range(std::make_pair(10000, 50000))
    , m_portType(EPortType::CLIENT)
{
    setType(ETopoType::PORT);
}

CPort::~CPort()
{
}

void CPort::setRange(unsigned short _min, unsigned short _max)
{
    m_range = make_pair(_min, _max);
}

const CPort::PortRange_t& CPort::getRange() const
{
    return m_range;
}

void CPort::setPortType(EPortType _portType)
{
    m_portType = _portType;
}

EPortType CPort::getPortType() const
{
    return m_portType;
}

void CPort::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& portPT = CTopoElement::findElement(ETopoType::PORT, _name, _pt.get_child("topology"));
        setName(portPT.get<string>("<xmlattr>.name"));
        setRange(portPT.get<unsigned int>("<xmlattr>.min"), portPT.get<unsigned int>("<xmlattr>.max"));
        setPortType(StringToPortType(portPT.get<string>("<xmlattr>.type")));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize port " + _name + " error:" + error.what());
    }
}

string CPort::toString() const
{
    stringstream ss;
    ss << "DDSPort: m_name=" << getName() << " m_range=(" << m_range.first << ", " << m_range.second << ")";
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CPort& _port)
{
    _strm << _port.toString();
    return _strm;
}
