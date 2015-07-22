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
        TopoElementPtr_t CreateTopoElement(ETopoType _type);

        TopoPropertyPtr_t CreateTopoProperty(ETopoType _type);

        TopoBasePtr_t CreateTopoBase(ETopoType _type);
    }
}
#endif /* defined(__DDS__TopoFactory__) */
