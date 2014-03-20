// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopoUtils.h"
// STD
#include <stdexcept>

using namespace std;

string DDSTopoTypeToTag(DDSTopoType _type)
{
    switch (_type)
    {
        case DDSTopoType::TASK:
            return "task";
        case DDSTopoType::COLLECTION:
            return "collection";
        case DDSTopoType::GROUP:
            return "group";
        case DDSTopoType::PORT:
            return "port";
        default:
            throw runtime_error("Topology element not found.");
    }
}

DDSTopoType DDSTagToTopoType(const string& _name)
{
    if (_name == "task")
        return DDSTopoType::TASK;
    else if (_name == "collection")
        return DDSTopoType::COLLECTION;
    else if (_name == "group")
        return DDSTopoType::GROUP;
    else if (_name == "port")
        return DDSTopoType::PORT;
    else
        throw runtime_error("Topology element with name " + _name + " does not exist.");
}
