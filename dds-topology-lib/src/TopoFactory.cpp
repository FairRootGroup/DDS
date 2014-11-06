// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoFactory.h"
#include "Task.h"
#include "TaskCollection.h"
#include "TaskGroup.h"
// STD
#include <memory>
#include <stdexcept>

using namespace std;

namespace dds
{
    TopoElementPtr_t CreateTopoElement(ETopoType _type)
    {
        switch (_type)
        {
            case ETopoType::TASK:
                return dynamic_pointer_cast<CTopoElement>(make_shared<CTask>());
            case ETopoType::COLLECTION:
                return dynamic_pointer_cast<CTopoElement>(make_shared<CTaskCollection>());
            case ETopoType::GROUP:
                return dynamic_pointer_cast<CTopoElement>(make_shared<CTaskGroup>());
            default:
                throw runtime_error("Topo element type does not exist.");
        }
    }

    TopoBasePtr_t CreateTopoBase(ETopoType _type)
    {
        switch (_type)
        {
            case ETopoType::TASK:
                return dynamic_pointer_cast<CTopoBase>(make_shared<CTask>());
            case ETopoType::COLLECTION:
                return dynamic_pointer_cast<CTopoBase>(make_shared<CTaskCollection>());
            case ETopoType::GROUP:
                return dynamic_pointer_cast<CTopoBase>(make_shared<CTaskGroup>());
            case ETopoType::TOPO_PROPERTY:
                return dynamic_pointer_cast<CTopoBase>(make_shared<CTopoProperty>());
            default:
                throw runtime_error("Topo base type does not exist.");
        }
    }
}
