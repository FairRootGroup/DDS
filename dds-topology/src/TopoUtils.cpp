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
    string TopoTypeToTag(ETopoType _type)
    {
        switch (_type)
        {
            case ETopoType::TASK:
                return "task";
            case ETopoType::COLLECTION:
                return "collection";
            case ETopoType::GROUP:
                return "group";
            case ETopoType::PORT:
                return "port";
            default:
                throw runtime_error("Topology element not found.");
        }
    }

    ETopoType TagToTopoType(const string& _name)
    {
        if (_name == "task")
            return ETopoType::TASK;
        else if (_name == "collection")
            return ETopoType::COLLECTION;
        else if (_name == "group")
            return ETopoType::GROUP;
        else if (_name == "port")
            return ETopoType::PORT;
        else
            throw runtime_error("Topology element with name " + _name + " does not exist.");
    }

    EPortType StringToPortType(const std::string& _name)
    {
        if (_name == "server")
            return EPortType::SERVER;
        else if (_name == "client")
            return EPortType::CLIENT;
        else
            throw runtime_error("Topology element with name " + _name + " does not exist.");
    }
}
