// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoUtils__
#define __DDS__TopoUtils__

// DDS
#include "Requirement.h"
#include "TopoElement.h"
#include "TopoProperty.h"
// STD
#include <string>

namespace dds
{
    namespace topology_api
    {
        std::string TopoTypeToUseTag(ETopoType _type);

        ETopoType UseTagToTopoType(const std::string& _name);

        std::string TopoTypeToDeclTag(ETopoType _type);

        ETopoType DeclTagToTopoType(const std::string& _name);

        EPropertyAccessType TagToPropertyAccessType(const std::string& _name);

        ERequirementType TagToRequirementType(const std::string& _name);
    }
}
#endif /* defined(__DDS__TopoUtils__) */
