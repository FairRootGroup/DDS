// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "DDSTask.h"
// STD
#include <sstream>
#include <string>
#include <iostream>
// BOOST
#include <boost/property_tree/ptree.hpp>

using namespace std;
using namespace boost::property_tree;

DDSTask::DDSTask()
    : DDSTopoElement()
    , m_exec()
    , m_ports()
{
    setType(DDSTopoElementType::TASK);
}

DDSTask::~DDSTask()
{
}

void DDSTask::setExec(const string& _exec)
{
    m_exec = _exec;
}

void DDSTask::setPorts(const DDSPortPtrVector_t& _ports)
{
    m_ports = _ports;
}

void DDSTask::addPort(DDSPortPtr_t& _port)
{
    m_ports.push_back(_port);
}

size_t DDSTask::getNofTasks() const
{
    return 1;
}

string DDSTask::getExec() const
{
    return m_exec;
}

size_t DDSTask::getNofPorts() const
{
    return m_ports.size();
}

DDSPortPtr_t DDSTask::getPort(size_t _i) const
{
    if (_i >= getNofPorts())
        throw out_of_range("Out of range exception");
    return m_ports[_i];
}

const DDSPortPtrVector_t& DDSTask::getPorts() const
{
    return m_ports;
}

void DDSTask::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& taskPT = DDSTopoElement::findElement("task", _name, _pt);

        setName(taskPT.get<string>("<xmlattr>.name"));
        setExec(taskPT.get<string>("<xmlattr>.exec"));
        for (const auto& port : taskPT)
        {
            if (port.first == "port")
            {
                DDSPortPtr_t newPort = make_shared<DDSPort>();
                newPort->initFromPropertyTree(port.second.get<string>("<xmlattr>.name"), _pt);
                addPort(newPort);
            }
        }
    }
    catch (ptree_error& error)
    {
        throw logic_error("Unable to initialize task " + _name + " error: " + error.what());
    }
    catch (logic_error& error)
    {
        throw logic_error("Unable to initialize task " + _name + " error: " + error.what());
    }
}

string DDSTask::toString() const
{
    stringstream ss;
    ss << "DDSTask: m_name=" << getName() << " m_exec=" << m_exec << " m_ports:\n";
    for (const auto& port : m_ports)
    {
        ss << " - " << port->toString() << endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const DDSTask& _task)
{
    _strm << _task.toString();
    return _strm;
}
