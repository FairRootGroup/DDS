// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "DDSPort.h"

DDSPort::DDSPort()
    : m_range(std::make_pair(10000, 50000))
{
}

DDSPort::~DDSPort()
{
}

void DDSPort::setName(const std::string& _name)
{
    m_name = _name;
}

void DDSPort::setRange(unsigned short _min, unsigned short _max)
{
    m_range = std::make_pair(_min, _max);
}

std::string DDSPort::getName() const
{
    return m_name;
}
const DDSPort::DDSPortRange_t& DDSPort::getRange() const
{
    return m_range;
}

/**
 * \brief Returns string representation of an object.
 * \return String representation of an object.
 */
std::string DDSPort::toString() const
{
    std::stringstream ss;
    ss << "DDSPort: m_name=" << m_name << " m_range=(" << m_range.first << ", " << m_range.second << ")";
    return ss.str();
}
