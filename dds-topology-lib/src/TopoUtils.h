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
// BOOST
#include <boost/property_tree/ptree.hpp>

namespace dds
{
    namespace topology_api
    {
        std::string TopoTypeToUseTag(CTopoBase::EType _type);

        CTopoBase::EType UseTagToTopoType(const std::string& _name);

        std::string TopoTypeToDeclTag(CTopoBase::EType _type);

        CTopoBase::EType DeclTagToTopoType(const std::string& _name);

        CTopoProperty::EAccessType TagToPropertyAccessType(const std::string& _name);

        std::string PropertyAccessTypeToTag(CTopoProperty::EAccessType _type);

        CTopoProperty::EScopeType TagToPropertyScopeType(const std::string& _name);

        std::string PropertyScopeTypeToTag(CTopoProperty::EScopeType _type);

        CTopoRequirement::EType TagToRequirementType(const std::string& _name);

        std::string RequirementTypeToTag(CTopoRequirement::EType _type);

        CTopoTrigger::EConditionType TagToConditionType(const std::string& _name);

        std::string ConditionTypeToTag(CTopoTrigger::EConditionType _type);

        CTopoTrigger::EActionType TagToActionType(const std::string& _name);

        std::string ActionTypeToTag(CTopoTrigger::EActionType _type);

        /// \brief Helper function to find element in property tree by type and name.
        /// \param[in] _type Type of the topo element we are looking for.
        /// \param[in] _name Name of element we are looking for.
        /// \param[in] _pt Property tree.
        /// \return Property tree with root node pointing to found element.
        /// \throw logic_error if element was not found.
        /// \note This function does not catch exceptions from property tree.
        const boost::property_tree::ptree& FindElementInPropertyTree(CTopoBase::EType _type,
                                                                     const std::string& _name,
                                                                     const boost::property_tree::ptree& _pt);

    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoUtils__) */
