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

void DDSTask::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    bool initialized = false;
    try
    {
        const ptree& topPT = _pt.get_child("topology");
        for (const auto& v : topPT)
        {
            if (v.first != "task")
                continue;

            const auto& taskPT = v.second;
            string taskName = taskPT.get<string>("<xmlattr>.name");
            if (taskName == _name)
            {
                setName(taskName);
                setExec(taskPT.get<string>("<xmlattr>.exec"));
                for (const auto& port : taskPT)
                {
                    if (port.first == "port")
                    {
                        DDSPortPtr_t newPort = make_shared<DDSPort>();
                        string portName = port.second.get<string>("<xmlattr>.name");
                        newPort->initFromPropertyTree(portName, _pt);
                        addPort(newPort);
                    }
                }

                initialized = true;
                break;
            }
        }
    }
    catch (ptree_error& error)
    {
        cout << "ptree_error: " << error.what() << endl;
    }

    if (!initialized)
        throw logic_error("Unable to initialize task " + _name);
}

string DDSTask::toString() const
{
    stringstream ss;
    ss << "DDSTask: m_name=" << getName() << " m_exec=" << m_exec << " m_ports:\n";
    for (const auto& port : m_ports)
    {
        ss << " - " << port->toString() << std::endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const DDSTask& _task)
{
    _strm << _task.toString();
    return _strm;
}
