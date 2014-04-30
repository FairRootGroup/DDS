// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoUtils__
#define __DDS__TopoUtils__

// DDS
#include "TopoElement.h"
#include "Port.h"
// STD
#include <string>

namespace dds
{
    std::string TopoTypeToTag(ETopoType _type);

    ETopoType TagToTopoType(const std::string& _name);

    EPortType StringToPortType(const std::string& _name);
}
#endif /* defined(__DDS__TopoUtils__) */
