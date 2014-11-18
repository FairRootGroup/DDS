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
            case ETopoType::RESTRICTION:
                return "restriction";
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
        else if (_name == "restriction")
            return ETopoType::RESTRICTION;
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
            case ETopoType::RESTRICTION:
                return "restriction";
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
        else if (_name == "restriction")
            return ETopoType::RESTRICTION;
        else
            throw runtime_error("Topology element with name " + _name + " does not exist.");
    }
}
