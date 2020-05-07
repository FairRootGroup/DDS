// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoTrigger.h"
#include "TopoUtils.h"
// STD
#include <boost/regex.hpp>
#include <iostream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTopoTrigger::CTopoTrigger(const std::string& _name)
    : CTopoBase(_name)
{
    setType(CTopoBase::EType::TRIGGER);
}

CTopoTrigger::~CTopoTrigger()
{
}

CTopoTrigger::EConditionType CTopoTrigger::getCondition() const
{
    return m_condition;
}

CTopoTrigger::EActionType CTopoTrigger::getAction() const
{
    return m_action;
}

const std::string& CTopoTrigger::getArgument() const
{
    return m_argument;
}

void CTopoTrigger::setAction(CTopoTrigger::EActionType _action)
{
    m_action = _action;
}

void CTopoTrigger::setCondition(CTopoTrigger::EConditionType _condition)
{
    m_condition = _condition;
}

void CTopoTrigger::setArgument(const std::string& _argument)
{
    m_argument = _argument;
}

void CTopoTrigger::initFromPropertyTree(const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& triggerPT =
            FindElementInPropertyTree(CTopoBase::EType::TRIGGER, getName(), _pt.get_child("topology"));
        setAction(TagToActionType(triggerPT.get<std::string>("<xmlattr>.action", "")));
        setCondition(TagToConditionType(triggerPT.get<std::string>("<xmlattr>.condition", "")));
        setArgument(triggerPT.get<std::string>("<xmlattr>.arg", ""));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize trigger " + getName() + " error:" + error.what());
    }
}

void CTopoTrigger::saveToPropertyTree(boost::property_tree::ptree& _pt)
{
    try
    {
        std::string tag("topology.decltrigger.<xmlattr>");
        _pt.put(tag + ".name", getName());
        _pt.put(tag + ".action", ActionTypeToTag(getAction()));
        _pt.put(tag + ".condition", ConditionTypeToTag(getCondition()));
        _pt.put(tag + ".arg", getArgument());
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to save trigger " + getName() + " error:" + error.what());
    }
}

string CTopoTrigger::toString() const
{
    stringstream ss;
    ss << "DDSTrigger: name=" << getName() << " action=" << ActionTypeToTag(getAction())
       << " condition=" << ConditionTypeToTag(getCondition()) << " arguments=" << getArgument();
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoTrigger& _requirement)
{
    _strm << _requirement.toString();
    return _strm;
}

string CTopoTrigger::hashString() const
{
    stringstream ss;
    ss << "|Trigger|" << getName() << "|" << ActionTypeToTag(getAction()) << "|" << ConditionTypeToTag(getCondition())
       << "|" << getArgument() << "|";
    return ss.str();
}
