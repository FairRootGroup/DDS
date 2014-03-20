// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopoFactory.h"
#include "DDSTask.h"
#include "DDSTaskCollection.h"
#include "DDSTaskGroup.h"
// STD
#include <memory>
#include <stdexcept>

using namespace std;

DDSTopoElementPtr_t DDSCreateTopoElement(DDSTopoType _type)
{
    switch (_type)
    {
        case DDSTopoType::TASK:
            return dynamic_pointer_cast<DDSTopoElement>(make_shared<DDSTask>());
        case DDSTopoType::COLLECTION:
            return dynamic_pointer_cast<DDSTopoElement>(make_shared<DDSTaskCollection>());
        case DDSTopoType::GROUP:
            return dynamic_pointer_cast<DDSTopoElement>(make_shared<DDSTaskGroup>());
        default:
            throw runtime_error("Topo element type does not exist.");
    }
}

DDSTopoPropertyPtr_t DDSCreateTopoProperty(DDSTopoType _type)
{
    switch (_type)
    {
        case DDSTopoType::PORT:
            return dynamic_pointer_cast<DDSTopoProperty>(make_shared<DDSPort>());
        default:
            throw runtime_error("Topo property type does not exist.");
    }
}

DDSTopoBasePtr_t DDSCreateTopoBase(DDSTopoType _type)
{
    switch (_type)
    {
        case DDSTopoType::TASK:
            return dynamic_pointer_cast<DDSTopoBase>(make_shared<DDSTask>());
        case DDSTopoType::COLLECTION:
            return dynamic_pointer_cast<DDSTopoBase>(make_shared<DDSTaskCollection>());
        case DDSTopoType::GROUP:
            return dynamic_pointer_cast<DDSTopoBase>(make_shared<DDSTaskGroup>());
        case DDSTopoType::PORT:
            return dynamic_pointer_cast<DDSTopoBase>(make_shared<DDSPort>());
        default:
            throw runtime_error("Topo base type does not exist.");
    }
}
