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

CTopoTask::CTopoTask(const string& _name)
    : CTopoElement(_name)
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

CTopoProperty::Ptr_t CTopoTask::addProperty(const string& _name)
{
    auto property{ make_shared<CTopoProperty>(_name) };
    property->setParent(this);
    m_properties.insert(make_pair(property->getName(), property));
    return property;
}

CTopoRequirement::Ptr_t CTopoTask::addRequirement(const string& _name)
{
    auto requirement{ make_shared<CTopoRequirement>(_name) };
    requirement->setParent(this);
    m_requirements.push_back(requirement);
    return requirement;
}

CTopoTrigger::Ptr_t CTopoTask::addTrigger(const string& _name)
{
    auto trigger{ make_shared<CTopoTrigger>(_name) };
    trigger->setParent(this);
    m_triggers.push_back(trigger);
    return trigger;
}

CTopoAsset::Ptr_t CTopoTask::addAsset(const string& _name)
{
    auto asset{ make_shared<CTopoAsset>(_name) };
    asset->setParent(this);
    m_assets.push_back(asset);
    return asset;
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

size_t CTopoTask::getNofAssets() const
{
    return m_assets.size();
}

size_t CTopoTask::getTotalCounter() const
{
    return getTotalCounterDefault();
}

CTopoProperty::Ptr_t CTopoTask::getProperty(const string& _id) const
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

const CTopoAsset::PtrVector_t& CTopoTask::getAssets() const
{
    return m_assets;
}

string CTopoTask::getParentCollectionId() const
{
    return (getParent() == nullptr || getParent()->getType() == CTopoBase::EType::GROUP) ? "" : getParent()->getName();
}

string CTopoTask::getParentGroupId() const
{
    if (getParent() == nullptr)
        return "";
    else if (getParent()->getType() == CTopoBase::EType::GROUP)
        return getParent()->getName();
    else if (getParent()->getParent() != nullptr)
        return getParent()->getParent()->getName();
    return "";
}

void CTopoTask::initFromPropertyTree(const ptree& _pt)
{
    try
    {
        const ptree& taskPT = FindElementInPropertyTree(CTopoBase::EType::TASK, getName(), _pt.get_child("topology"));
        setExe(taskPT.get<string>("exe"));
        setEnv(taskPT.get<string>("env", ""));
        setExeReachable(taskPT.get<bool>("exe.<xmlattr>.reachable", true));
        setEnvReachable(taskPT.get<bool>("env.<xmlattr>.reachable", true));

        boost::optional<const ptree&> requirementsPT = taskPT.get_child_optional("requirements");
        if (requirementsPT)
        {
            for (const auto& requirement : requirementsPT.get())
            {
                addRequirement(requirement.second.data())->initFromPropertyTree(_pt);
            }
        }

        boost::optional<const ptree&> propertiesPT = taskPT.get_child_optional("properties");
        if (propertiesPT)
        {
            for (const auto& property : propertiesPT.get())
            {
                auto newProperty{ addProperty(property.second.data()) };
                newProperty->initFromPropertyTree(_pt);
                newProperty->setAccessType(TagToPropertyAccessType(property.second.get<string>("<xmlattr>.access")));
            }
        }

        boost::optional<const ptree&> triggersPT = taskPT.get_child_optional("triggers");
        if (triggersPT)
        {
            for (const auto& trigger : triggersPT.get())
            {
                addTrigger(trigger.second.data())->initFromPropertyTree(_pt);
            }
        }

        boost::optional<const ptree&> assetsPT = taskPT.get_child_optional("assets");
        if (assetsPT)
        {
            for (const auto& asset : assetsPT.get())
            {
                addAsset(asset.second.data())->initFromPropertyTree(_pt);
            }
        }
    }
    catch (exception& error) // ptree_error, logic_error
    {
        throw logic_error("Unable to initialize task " + getName() + " error: " + error.what());
    }
}

void CTopoTask::saveToPropertyTree(ptree& _pt)
{
    try
    {
        string tag("topology.decltask");
        _pt.put(tag + ".<xmlattr>.name", getName());
        _pt.put(tag + ".exe", getExe());
        _pt.put(tag + ".exe.<xmlattr>.reachable", isExeReachable());
        if (getEnv().length() != 0)
        {
            _pt.put(tag + ".env", getEnv());
            _pt.put(tag + ".env.<xmlattr>.reachable", isEnvReachable());
        }

        for (const auto& v : m_requirements)
        {
            _pt.add(tag + ".requirements.name", v->getName());
        }

        for (const auto& v : m_properties)
        {
            boost::property_tree::ptree pt;
            pt.put_value(v.first);
            pt.add("<xmlattr>.access", PropertyAccessTypeToTag(v.second->getAccessType()));
            _pt.add_child(tag + ".properties.name", pt);
        }

        for (const auto& v : m_triggers)
        {
            _pt.add(tag + ".triggers.name", v->getName());
        }

        for (const auto& v : m_assets)
        {
            _pt.add(tag + ".assets.name", v->getName());
        }
    }
    catch (exception& error) // ptree_error, logic_error
    {
        throw logic_error("Unable to save task " + getName() + " error: " + error.what());
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
    ss << " m_assets:\n";
    for (const auto& asset : m_assets)
    {
        ss << " - " << asset->toString() << endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoTask& _task)
{
    _strm << _task.toString();
    return _strm;
}

string CTopoTask::hashString() const
{
    stringstream ss;
    ss << "|Task|" << getName() << "|" << getExe() << "|" << getEnv() << "|" << isExeReachable() << "|"
       << isEnvReachable();
    for (const auto& property : getProperties())
    {
        ss << property.second->hashString() << "|";
    }
    for (const auto& requirement : getRequirements())
    {
        ss << requirement->hashString() << "|";
    }
    for (const auto& trigger : getTriggers())
    {
        ss << trigger->hashString() << "|";
    }
    for (const auto& asset : getAssets())
    {
        ss << asset->hashString() << "|";
    }
    return ss.str();
}
