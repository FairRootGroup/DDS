// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Trigger.h"
#include "TopoUtils.h"
// STD
#include <boost/regex.hpp>
#include <iostream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTrigger::CTrigger()
    : CTopoBase()
    , m_action()
    , m_condition(EConditionType::None)
{
    setType(ETopoType::TRIGGER);
}

CTrigger::~CTrigger()
{
}

EConditionType CTrigger::getCondition() const
{
    return m_condition;
}

EActionType CTrigger::getAction() const
{
    return m_action;
}

const std::string& CTrigger::getArgument() const
{
    return m_argument;
}

void CTrigger::setAction(EActionType _action)
{
    m_action = _action;
}

void CTrigger::setCondition(EConditionType _condition)
{
    m_condition = _condition;
}

void CTrigger::setArgument(const std::string& _argument)
{
    m_argument = _argument;
}

void CTrigger::initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt)
{
    try
    {
        const ptree& triggerPT = CTopoBase::findElement(ETopoType::TRIGGER, _name, _pt.get_child("topology"));
        setId(triggerPT.get<string>("<xmlattr>.id"));
        setAction(TagToActionType(triggerPT.get<std::string>("<xmlattr>.action", "")));
        setCondition(TagToConditionType(triggerPT.get<std::string>("<xmlattr>.condition", "")));
        setArgument(triggerPT.get<std::string>("<xmlattr>.arg", ""));
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize trigger " + _name + " error:" + error.what());
    }
}

string CTrigger::toString() const
{
    stringstream ss;
    ss << "DDSTrigger: id=" << getId() << " action=" << ActionTypeToTag(getAction())
       << " condition=" << ConditionTypeToTag(getCondition()) << " arguments=" << getArgument();
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTrigger& _requirement)
{
    _strm << _requirement.toString();
    return _strm;
}
