// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoUtils.h"
// STD
#include <stdexcept>

using namespace std;
namespace dds
{
    namespace topology_api
    {
        string TopoTypeToUseTag(ETopoType _type)
        {
            switch (_type)
            {
                case ETopoType::TASK:
                    return "task";
                case ETopoType::COLLECTION:
                    return "collection";
                case ETopoType::GROUP:
                    return "group";
                case ETopoType::TOPO_PROPERTY:
                    return "property";
                case ETopoType::REQUIREMENT:
                    return "requirement";
                case ETopoType::TRIGGER:
                    return "trigger";
                default:
                    throw runtime_error("Topology element not found.");
            }
        }

        ETopoType UseTagToTopoType(const string& _name)
        {
            if (_name == "task")
                return ETopoType::TASK;
            else if (_name == "collection")
                return ETopoType::COLLECTION;
            else if (_name == "group")
                return ETopoType::GROUP;
            else if (_name == "property")
                return ETopoType::TOPO_PROPERTY;
            else if (_name == "requirement")
                return ETopoType::REQUIREMENT;
            else if (_name == "trigger")
                return ETopoType::TRIGGER;
            else
                throw runtime_error("Topology element with name " + _name + " does not exist.");
        }

        string TopoTypeToDeclTag(ETopoType _type)
        {
            switch (_type)
            {
                case ETopoType::TASK:
                    return "decltask";
                case ETopoType::COLLECTION:
                    return "declcollection";
                case ETopoType::GROUP:
                    return "group";
                case ETopoType::TOPO_PROPERTY:
                    return "property";
                case ETopoType::REQUIREMENT:
                    return "declrequirement";
                case ETopoType::TOPO_VARS:
                    return "var";
                case ETopoType::TRIGGER:
                    return "decltrigger";
                default:
                    throw runtime_error("Topology element not found.");
            }
        }

        ETopoType DeclTagToTopoType(const string& _name)
        {
            if (_name == "decltask")
                return ETopoType::TASK;
            else if (_name == "declcollection")
                return ETopoType::COLLECTION;
            else if (_name == "group")
                return ETopoType::GROUP;
            else if (_name == "property")
                return ETopoType::TOPO_PROPERTY;
            else if (_name == "declrequirement")
                return ETopoType::REQUIREMENT;
            else if (_name == "var")
                return ETopoType::TOPO_VARS;
            else if (_name == "decltrigger")
                return ETopoType::TRIGGER;
            else
                throw runtime_error("Topology element with name " + _name + " does not exist.");
        }

        EPropertyAccessType TagToPropertyAccessType(const string& _name)
        {
            if (_name == "read")
                return EPropertyAccessType::READ;
            if (_name == "write")
                return EPropertyAccessType::WRITE;
            if (_name == "readwrite")
                return EPropertyAccessType::READWRITE;
            else
                throw runtime_error("Property access type with name " + _name + " does not exist.");
        }

        ERequirementType TagToRequirementType(const string& _name)
        {
            if (_name == "wnname")
                return ERequirementType::WnName;
            if (_name == "hostname")
                return ERequirementType::HostName;
            if (_name == "gpu")
                return ERequirementType::Gpu;
            else
                throw runtime_error("Host pattern type with name " + _name + " does not exist.");
        }

        std::string RequirementTypeToTag(ERequirementType _type)
        {
            switch (_type)
            {
                case ERequirementType::WnName:
                    return "WnName";
                case ERequirementType::HostName:
                    return "HostName";
                case ERequirementType::Gpu:
                    return "Gpu";
                default:
                    throw runtime_error("Topology element not found.");
            }
        }

        EConditionType TagToConditionType(const std::string& _name)
        {
            if (_name == "TaskCrashed")
                return EConditionType::TaskCrashed;
            else
                throw runtime_error("Condition type with name " + _name + " does not exist.");
        }

        std::string ConditionTypeToTag(EConditionType _type)
        {
            switch (_type)
            {
                case EConditionType::TaskCrashed:
                    return "TaskCrashed";
                default:
                    throw runtime_error("Topology element not found.");
            }
        }

        EActionType TagToActionType(const std::string& _name)
        {
            if (_name == "RestartTask")
                return EActionType::RestartTask;
            else
                throw runtime_error("Action type with name " + _name + " does not exist.");
        }

        std::string ActionTypeToTag(EActionType _type)
        {
            switch (_type)
            {
                case EActionType::RestartTask:
                    return "RestartTask";
                default:
                    throw runtime_error("Topology element not found.");
            }
        }
    } // namespace topology_api
} // namespace dds
