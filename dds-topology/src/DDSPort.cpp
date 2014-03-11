// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "DDSPort.h"

using namespace std;

DDSPort::DDSPort()
    : m_range(make_pair(10000, 50000))
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