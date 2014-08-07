// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "Task.h"
#include "TaskGroup.h"
// STD
#include <sstream>
#include <string>
#include <iostream>
#include <memory>
// BOOST
#include <boost/property_tree/ptree.hpp>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

CTask::CTask()
    : CTopoElement()
    , m_exec()
    , m_env()
    , m_ports()
{
    setType(ETopoType::TASK);
}

CTask::~CTask()
{
}

void CTask::setExec(const string& _exec)
{
    m_exec = _exec;
}

void CTask::setEnv(const string& _env)
{
    m_env = _env;
}

void CTask::setPorts(const PortPtrVector_t& _ports)
{
    m_ports = _ports;
}

void CTask::addPort(PortPtr_t& _port)
{
    m_ports.push_back(_port);
}

size_t CTask::getNofTasks() const
{
    return 1;
}

size_t CTask::getTotalNofTasks() const
{
    return 1;
}

const string& CTask::getExec() const
{
    return m_exec;
}

const string& CTask::getEnv() const
{
    return m_env;
}

size_t CTask::getNofPorts() const
{
    return m_ports.size();
}

size_t CTask::getTotalCounter() const
{
    return getTotalCounterDefault();
}

PortPtr_t CTask::getPort(size_t _i) const
{
    if (_i >= getNofPorts())
        throw out_of_range("Out of range exception");
    return m_ports[_i];
}

const PortPtrVector_t& CTask::getPorts() const
{
    return m_ports;
}

void CTask::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& taskPT = CTopoElement::findElement(ETopoType::TASK, _name, _pt.get_child("topology"));

        setName(taskPT.get<string>("<xmlattr>.name"));
        setExec(taskPT.get<string>("<xmlattr>.exec"));
        setEnv(taskPT.get<string>("<xmlattr>.env", ""));
        for (const auto& port : taskPT)
        {
            if (port.first == "<xmlattr>")
                continue;
            PortPtr_t newPort = make_shared<CPort>();
            newPort->setParent(this);
            newPort->initFromPropertyTree(port.second.get<string>("<xmlattr>.name"), _pt);
            addPort(newPort);
        }
    }
    catch (exception& error) // ptree_error, logic_error
    {
        throw logic_error("Unable to initialize task " + _name + " error: " + error.what());
    }
}

string CTask::toString() const
{
    stringstream ss;
    ss << "Task: m_name=" << getName() << " m_exec=" << m_exec << " m_ports:\n";
    for (const auto& port : m_ports)
    {
        ss << " - " << port->toString() << endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTask& _task)
{
    _strm << _task.toString();
    return _strm;
}
