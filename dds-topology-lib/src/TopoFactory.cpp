// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoFactory.h"
#include "TopoCollection.h"
#include "TopoGroup.h"
#include "TopoTask.h"
// STD
#include <memory>
#include <stdexcept>

using namespace std;

namespace dds
{
    namespace topology_api
    {

        CTopoElement::Ptr_t CreateTopoElement(CTopoBase::EType _type)
        {
            switch (_type)
            {
                case CTopoBase::EType::TASK:
                    return dynamic_pointer_cast<CTopoElement>(make_shared<CTopoTask>());
                case CTopoBase::EType::COLLECTION:
                    return dynamic_pointer_cast<CTopoElement>(make_shared<CTopoCollection>());
                case CTopoBase::EType::GROUP:
                    return dynamic_pointer_cast<CTopoElement>(make_shared<CTopoGroup>());
                default:
                    throw runtime_error("Topo element type does not exist.");
            }
        }

        CTopoBase::Ptr_t CreateTopoBase(CTopoBase::EType _type)
        {
            switch (_type)
            {
                case CTopoBase::EType::TASK:
                    return dynamic_pointer_cast<CTopoBase>(make_shared<CTopoTask>());
                case CTopoBase::EType::COLLECTION:
                    return dynamic_pointer_cast<CTopoBase>(make_shared<CTopoCollection>());
                case CTopoBase::EType::GROUP:
                    return dynamic_pointer_cast<CTopoBase>(make_shared<CTopoGroup>());
                case CTopoBase::EType::TOPO_PROPERTY:
                    return dynamic_pointer_cast<CTopoBase>(make_shared<CTopoProperty>());
                default:
                    throw runtime_error("Topo base type does not exist.");
            }
        }
    } // namespace topology_api
} // namespace dds
