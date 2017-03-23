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
#include "Trigger.h"
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

        std::string RequirementTypeToTag(ERequirementType _type);

        EConditionType TagToConditionType(const std::string& _name);

        std::string ConditionTypeToTag(EConditionType _type);

        EActionType TagToActionType(const std::string& _name);

        std::string ActionTypeToTag(EActionType _type);
    }
}
#endif /* defined(__DDS__TopoUtils__) */
