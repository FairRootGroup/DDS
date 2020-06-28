// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoUtils.h"
// STD
#include <stdexcept>

using namespace std;
using namespace boost::property_tree;

namespace dds
{
    namespace topology_api
    {
        string TopoTypeToUseTag(CTopoBase::EType _type)
        {
            switch (_type)
            {
                case CTopoBase::EType::TASK:
                    return "task";
                case CTopoBase::EType::COLLECTION:
                    return "collection";
                case CTopoBase::EType::GROUP:
                    return "group";
                case CTopoBase::EType::TOPO_PROPERTY:
                    return "property";
                case CTopoBase::EType::REQUIREMENT:
                    return "requirement";
                case CTopoBase::EType::TRIGGER:
                    return "trigger";
                default:
                    throw runtime_error("Topology element not found.");
            }
        }

        CTopoBase::EType UseTagToTopoType(const string& _name)
        {
            if (_name == "task")
                return CTopoBase::EType::TASK;
            else if (_name == "collection")
                return CTopoBase::EType::COLLECTION;
            else if (_name == "group")
                return CTopoBase::EType::GROUP;
            else if (_name == "property")
                return CTopoBase::EType::TOPO_PROPERTY;
            else if (_name == "requirement")
                return CTopoBase::EType::REQUIREMENT;
            else if (_name == "trigger")
                return CTopoBase::EType::TRIGGER;
            else
                throw runtime_error("Topology element with name " + _name + " does not exist.");
        }

        string TopoTypeToDeclTag(CTopoBase::EType _type)
        {
            switch (_type)
            {
                case CTopoBase::EType::TASK:
                    return "decltask";
                case CTopoBase::EType::COLLECTION:
                    return "declcollection";
                case CTopoBase::EType::GROUP:
                    return "group";
                case CTopoBase::EType::TOPO_PROPERTY:
                    return "property";
                case CTopoBase::EType::REQUIREMENT:
                    return "declrequirement";
                case CTopoBase::EType::TOPO_VARS:
                    return "var";
                case CTopoBase::EType::TRIGGER:
                    return "decltrigger";
                default:
                    throw runtime_error("Topology element not found.");
            }
        }

        CTopoBase::EType DeclTagToTopoType(const string& _name)
        {
            if (_name == "decltask")
                return CTopoBase::EType::TASK;
            else if (_name == "declcollection")
                return CTopoBase::EType::COLLECTION;
            else if (_name == "group")
                return CTopoBase::EType::GROUP;
            else if (_name == "property")
                return CTopoBase::EType::TOPO_PROPERTY;
            else if (_name == "declrequirement")
                return CTopoBase::EType::REQUIREMENT;
            else if (_name == "var")
                return CTopoBase::EType::TOPO_VARS;
            else if (_name == "decltrigger")
                return CTopoBase::EType::TRIGGER;
            else
                throw runtime_error("Topology element with name " + _name + " does not exist.");
        }

        CTopoProperty::EAccessType TagToPropertyAccessType(const string& _name)
        {
            if (_name == "read")
                return CTopoProperty::EAccessType::READ;
            if (_name == "write")
                return CTopoProperty::EAccessType::WRITE;
            if (_name == "readwrite")
                return CTopoProperty::EAccessType::READWRITE;
            else
                throw runtime_error("Property access type with name " + _name + " does not exist.");
        }

        std::string PropertyAccessTypeToTag(CTopoProperty::EAccessType _type)
        {
            switch (_type)
            {
                case CTopoProperty::EAccessType::READ:
                    return "read";
                case CTopoProperty::EAccessType::WRITE:
                    return "write";
                case CTopoProperty::EAccessType::READWRITE:
                    return "readwrite";
                default:
                    throw runtime_error("Property access type not found");
            }
        }

        CTopoProperty::EScopeType TagToPropertyScopeType(const std::string& _name)
        {
            if (_name == "collection")
                return CTopoProperty::EScopeType::COLLECTION;
            if (_name == "global")
                return CTopoProperty::EScopeType::GLOBAL;
            else
                throw runtime_error("Property scope type with name " + _name + " does not exist.");
        }

        std::string PropertyScopeTypeToTag(CTopoProperty::EScopeType _type)
        {
            switch (_type)
            {
                case CTopoProperty::EScopeType::COLLECTION:
                    return "collection";
                case CTopoProperty::EScopeType::GLOBAL:
                    return "global";
                default:
                    throw runtime_error("Property scope not found.");
            }
        }

        CTopoRequirement::EType TagToRequirementType(const string& _name)
        {
            if (_name == "wnname")
                return CTopoRequirement::EType::WnName;
            if (_name == "hostname")
                return CTopoRequirement::EType::HostName;
            if (_name == "gpu")
                return CTopoRequirement::EType::Gpu;
            if (_name == "maxinstances")
                return CTopoRequirement::EType::MaxInstancesPerHost;
            else
                throw runtime_error("Host pattern type with name " + _name + " does not exist.");
        }

        std::string RequirementTypeToTag(CTopoRequirement::EType _type)
        {
            switch (_type)
            {
                case CTopoRequirement::EType::WnName:
                    return "wnname";
                case CTopoRequirement::EType::HostName:
                    return "hostname";
                case CTopoRequirement::EType::Gpu:
                    return "gpu";
                case CTopoRequirement::EType::MaxInstancesPerHost:
                    return "maxinstances";
                default:
                    throw runtime_error("Topology element not found.");
            }
        }

        CTopoTrigger::EConditionType TagToConditionType(const std::string& _name)
        {
            if (_name == "TaskCrashed")
                return CTopoTrigger::EConditionType::TaskCrashed;
            else
                throw runtime_error("Condition type with name " + _name + " does not exist.");
        }

        std::string ConditionTypeToTag(CTopoTrigger::EConditionType _type)
        {
            switch (_type)
            {
                case CTopoTrigger::EConditionType::TaskCrashed:
                    return "TaskCrashed";
                default:
                    throw runtime_error("Topology element not found.");
            }
        }

        CTopoTrigger::EActionType TagToActionType(const std::string& _name)
        {
            if (_name == "RestartTask")
                return CTopoTrigger::EActionType::RestartTask;
            else
                throw runtime_error("Action type with name " + _name + " does not exist.");
        }

        std::string ActionTypeToTag(CTopoTrigger::EActionType _type)
        {
            switch (_type)
            {
                case CTopoTrigger::EActionType::RestartTask:
                    return "RestartTask";
                default:
                    throw runtime_error("Topology element not found.");
            }
        }

        const ptree& FindElementInPropertyTree(CTopoBase::EType _type, const string& _name, const ptree& _pt)
        {
            const ptree* result = nullptr;
            for (const auto& v : _pt)
            {
                const auto& elementPT = v.second;
                if (v.first == TopoTypeToDeclTag(_type) && elementPT.get<string>("<xmlattr>.name") == _name)
                {
                    if (result != nullptr)
                        throw logic_error("Element \"" + _name + "\" has dublicated name.");
                    result = &elementPT;
                }
            }
            if (result == nullptr)
                throw logic_error("Element \"" + _name + "\"not found in property tree.");

            return *result;
        }

    } // namespace topology_api
} // namespace dds
