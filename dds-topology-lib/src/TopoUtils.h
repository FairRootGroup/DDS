// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoUtils__
#define __DDS__TopoUtils__

// DDS
#include "TopoElement.h"
#include "TopoProperty.h"
#include "TopoRequirement.h"
#include "TopoTrigger.h"
// STD
#include <string>

namespace dds
{
    namespace topology_api
    {
        std::string TopoTypeToUseTag(CTopoBase::EType _type);

        CTopoBase::EType UseTagToTopoType(const std::string& _name);

        std::string TopoTypeToDeclTag(CTopoBase::EType _type);

        CTopoBase::EType DeclTagToTopoType(const std::string& _name);

        CTopoProperty::EAccessType TagToPropertyAccessType(const std::string& _name);

        CTopoProperty::EScopeType TagToPropertyScopeType(const std::string& _name);

        CTopoRequirement::EType TagToRequirementType(const std::string& _name);

        std::string RequirementTypeToTag(CTopoRequirement::EType _type);

        CTopoTrigger::EConditionType TagToConditionType(const std::string& _name);

        std::string ConditionTypeToTag(CTopoTrigger::EConditionType _type);

        CTopoTrigger::EActionType TagToActionType(const std::string& _name);

        std::string ActionTypeToTag(CTopoTrigger::EActionType _type);
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoUtils__) */
