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
    , m_properties()
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

void CTask::setProperties(const TopoPropertyPtrVector_t& _properties)
{
    m_properties = _properties;
}

void CTask::addProperty(TopoPropertyPtr_t& _property)
{
    m_properties.push_back(_property);
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

size_t CTask::getNofProperties() const
{
    return m_properties.size();
}

size_t CTask::getTotalCounter() const
{
    return getTotalCounterDefault();
}

TopoPropertyPtr_t CTask::getProperty(size_t _i) const
{
    if (_i >= getNofProperties())
        throw out_of_range("Out of range exception");
    return m_properties[_i];
}

const TopoPropertyPtrVector_t& CTask::getProperties() const
{
    return m_properties;
}

void CTask::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& taskPT = CTopoElement::findElement(ETopoType::TASK, _name, _pt.get_child("topology"));

        setName(taskPT.get<string>("<xmlattr>.name"));
        setExec(taskPT.get<string>("<xmlattr>.exec"));
        setEnv(taskPT.get<string>("<xmlattr>.env", ""));
        for (const auto& property : taskPT)
        {
            if (property.first == "<xmlattr>")
                continue;
            TopoPropertyPtr_t newProperty = make_shared<CTopoProperty>();
            newProperty->setParent(this);
            newProperty->initFromPropertyTree(property.second.get<string>("<xmlattr>.name"), _pt);
            addProperty(newProperty);
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
    ss << "Task: m_name=" << getName() << " m_exec=" << m_exec << " m_env=" << m_env << " m_properties:\n";
    for (const auto& property : m_properties)
    {
        ss << " - " << property->toString() << endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTask& _task)
{
    _strm << _task.toString();
    return _strm;
}
