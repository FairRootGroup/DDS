// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "Task.h"
#include "TaskGroup.h"
#include "TopoUtils.h"
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
using namespace topology_api;

CTask::CTask()
    : CTopoElement()
    , m_exe()
    , m_env()
    , m_exeReachable(true)
    , m_envReachable(true)
    , m_properties()
{
    setType(ETopoType::TASK);
}

CTask::~CTask()
{
}

void CTask::setExe(const string& _exe)
{
    m_exe = _exe;
}

void CTask::setEnv(const string& _env)
{
    m_env = _env;
}

void CTask::setExeReachable(bool _exeReachable)
{
    m_exeReachable = _exeReachable;
}

void CTask::setEnvReachable(bool _envReachable)
{
    m_envReachable = _envReachable;
}

void CTask::setProperties(const TopoPropertyPtrVector_t& _properties)
{
    m_properties = _properties;
}

void CTask::addProperty(TopoPropertyPtr_t _property)
{
    m_properties.push_back(_property);
}

void CTask::setRequirement(RequirementPtr_t _requirement)
{
    m_requirement = _requirement;
}

size_t CTask::getNofTasks() const
{
    return 1;
}

size_t CTask::getTotalNofTasks() const
{
    return 1;
}

const string& CTask::getExe() const
{
    return m_exe;
}

const string& CTask::getEnv() const
{
    return m_env;
}

bool CTask::isExeReachable() const
{
    return m_exeReachable;
}

bool CTask::isEnvReachable() const
{
    return m_envReachable;
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

TopoPropertyPtr_t CTask::getProperty(const std::string& _id) const
{
    for (const auto& v : m_properties)
    {
        if (v->getId() == _id)
            return v;
    }
    return nullptr;
}

const TopoPropertyPtrVector_t& CTask::getProperties() const
{
    return m_properties;
}

RequirementPtr_t CTask::getRequirement() const
{
    return m_requirement;
}

std::string CTask::getParentCollectionId() const
{
    return (getParent() == nullptr || getParent()->getType() == ETopoType::GROUP) ? "" : getParent()->getId();
}

std::string CTask::getParentGroupId() const
{
    if (getParent() == nullptr)
        return "";
    else if (getParent()->getType() == ETopoType::GROUP)
        return getParent()->getId();
    else if (getParent()->getParent() != nullptr)
        return getParent()->getParent()->getId();
    return "";
}

void CTask::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& taskPT = CTopoElement::findElement(ETopoType::TASK, _name, _pt.get_child("topology"));

        setId(taskPT.get<string>("<xmlattr>.id"));
        setExe(taskPT.get<string>("exe"));
        setEnv(taskPT.get<string>("env", ""));
        setExeReachable(taskPT.get<bool>("exe.<xmlattr>.reachable", true));
        setEnvReachable(taskPT.get<bool>("env.<xmlattr>.reachable", true));

        string requirementId = taskPT.get<string>(TopoTypeToUseTag(ETopoType::REQUIREMENT), "");
        if (!requirementId.empty())
        {
            RequirementPtr_t newRequirement = make_shared<CRequirement>();
            newRequirement->setParent(this);
            newRequirement->initFromPropertyTree(requirementId, _pt);
            setRequirement(newRequirement);
        }

        boost::optional<const ptree&> propertiesPT = taskPT.get_child_optional("properties");
        if (propertiesPT)
        {
            for (const auto& property : propertiesPT.get())
            {
                TopoPropertyPtr_t newProperty = make_shared<CTopoProperty>();
                newProperty->setParent(this);
                newProperty->initFromPropertyTree(property.second.data(), _pt);
                newProperty->setAccessType(TagToPropertyAccessType(property.second.get<string>("<xmlattr>.access")));
                addProperty(newProperty);
            }
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
    ss << "Task: m_id=" << getId() << " m_exe=" << m_exe << " m_env=" << m_env << " m_properties:\n";
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
