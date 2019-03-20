// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "TopoTask.h"
#include "TopoGroup.h"
#include "TopoUtils.h"
// STD
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
// BOOST
#include <boost/property_tree/ptree.hpp>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTopoTask::CTopoTask()
    : CTopoElement()
    , m_exe()
    , m_env()
    , m_exeReachable(true)
    , m_envReachable(true)
    , m_properties()
{
    setType(CTopoBase::EType::TASK);
}

CTopoTask::~CTopoTask()
{
}

void CTopoTask::setExe(const string& _exe)
{
    m_exe = _exe;
}

void CTopoTask::setEnv(const string& _env)
{
    m_env = _env;
}

void CTopoTask::setExeReachable(bool _exeReachable)
{
    m_exeReachable = _exeReachable;
}

void CTopoTask::setEnvReachable(bool _envReachable)
{
    m_envReachable = _envReachable;
}

void CTopoTask::setProperties(const CTopoProperty::PtrMap_t& _properties)
{
    m_properties = _properties;
}

void CTopoTask::addProperty(CTopoProperty::Ptr_t _property)
{
    m_properties.insert(make_pair(_property->getName(), _property));
}

void CTopoTask::setRequirements(const CTopoRequirement::PtrVector_t& _requirements)
{
    m_requirements = _requirements;
}

void CTopoTask::addRequirement(CTopoRequirement::Ptr_t _requirement)
{
    m_requirements.push_back(_requirement);
}

void CTopoTask::setTriggers(const CTopoTrigger::PtrVector_t& _triggers)
{
    m_triggers = _triggers;
}

void CTopoTask::addTrigger(CTopoTrigger::Ptr_t _trigger)
{
    m_triggers.push_back(_trigger);
}

size_t CTopoTask::getNofTasks() const
{
    return 1;
}

size_t CTopoTask::getTotalNofTasks() const
{
    return 1;
}

const string& CTopoTask::getExe() const
{
    return m_exe;
}

const string& CTopoTask::getEnv() const
{
    return m_env;
}

bool CTopoTask::isExeReachable() const
{
    return m_exeReachable;
}

bool CTopoTask::isEnvReachable() const
{
    return m_envReachable;
}

size_t CTopoTask::getNofProperties() const
{
    return m_properties.size();
}

size_t CTopoTask::getNofRequirements() const
{
    return m_requirements.size();
}

size_t CTopoTask::getNofTriggers() const
{
    return m_triggers.size();
}

size_t CTopoTask::getTotalCounter() const
{
    return getTotalCounterDefault();
}

CTopoProperty::Ptr_t CTopoTask::getProperty(const std::string& _id) const
{
    auto it = m_properties.find(_id);
    if (it == m_properties.end())
        return nullptr;
    return it->second;
}

const CTopoProperty::PtrMap_t& CTopoTask::getProperties() const
{
    return m_properties;
}

const CTopoRequirement::PtrVector_t& CTopoTask::getRequirements() const
{
    return m_requirements;
}

const CTopoTrigger::PtrVector_t& CTopoTask::getTriggers() const
{
    return m_triggers;
}

std::string CTopoTask::getParentCollectionId() const
{
    return (getParent() == nullptr || getParent()->getType() == CTopoBase::EType::GROUP) ? "" : getParent()->getName();
}

std::string CTopoTask::getParentGroupId() const
{
    if (getParent() == nullptr)
        return "";
    else if (getParent()->getType() == CTopoBase::EType::GROUP)
        return getParent()->getName();
    else if (getParent()->getParent() != nullptr)
        return getParent()->getParent()->getName();
    return "";
}

void CTopoTask::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& taskPT = FindElementInPropertyTree(CTopoBase::EType::TASK, _name, _pt.get_child("topology"));

        setName(taskPT.get<string>("<xmlattr>.name"));
        setExe(taskPT.get<string>("exe"));
        setEnv(taskPT.get<string>("env", ""));
        setExeReachable(taskPT.get<bool>("exe.<xmlattr>.reachable", true));
        setEnvReachable(taskPT.get<bool>("env.<xmlattr>.reachable", true));

        boost::optional<const ptree&> requirementsPT = taskPT.get_child_optional("requirements");
        if (requirementsPT)
        {
            for (const auto& requirement : requirementsPT.get())
            {
                CTopoRequirement::Ptr_t newRequirement = make_shared<CTopoRequirement>();
                newRequirement->setParent(this);
                newRequirement->initFromPropertyTree(requirement.second.data(), _pt);
                addRequirement(newRequirement);
            }
        }

        boost::optional<const ptree&> propertiesPT = taskPT.get_child_optional("properties");
        if (propertiesPT)
        {
            for (const auto& property : propertiesPT.get())
            {
                CTopoProperty::Ptr_t newProperty = make_shared<CTopoProperty>();
                newProperty->setParent(this);
                newProperty->initFromPropertyTree(property.second.data(), _pt);
                newProperty->setAccessType(TagToPropertyAccessType(property.second.get<string>("<xmlattr>.access")));
                addProperty(newProperty);
            }
        }

        boost::optional<const ptree&> triggersPT = taskPT.get_child_optional("triggers");
        if (triggersPT)
        {
            for (const auto& trigger : triggersPT.get())
            {
                CTopoTrigger::Ptr_t newTrigger = make_shared<CTopoTrigger>();
                newTrigger->setParent(this);
                newTrigger->initFromPropertyTree(trigger.second.data(), _pt);
                addTrigger(newTrigger);
            }
        }
    }
    catch (exception& error) // ptree_error, logic_error
    {
        throw logic_error("Unable to initialize task " + _name + " error: " + error.what());
    }
}

string CTopoTask::toString() const
{
    stringstream ss;
    ss << "Task: m_name=" << getName() << " m_exe=" << m_exe << " m_env=" << m_env << " m_properties:\n";
    for (const auto& property : m_properties)
    {
        ss << " - " << property.second->toString() << endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoTask& _task)
{
    _strm << _task.toString();
    return _strm;
}
