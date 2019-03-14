// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoFactory__
#define __DDS__TopoFactory__

#include "TopoBase.h"
#include "TopoElement.h"
#include "TopoProperty.h"

namespace dds
{
    namespace topology_api
    {
        CTopoElement::Ptr_t CreateTopoElement(CTopoBase::EType _type);

        CTopoProperty::Ptr_t CreateTopoProperty(CTopoBase::EType _type);

        CTopoBase::Ptr_t CreateTopoBase(CTopoBase::EType _type);
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoFactory__) */
