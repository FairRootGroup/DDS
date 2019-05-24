// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoRequirement__
#define __DDS__TopoRequirement__

// DDS
#include "TopoBase.h"
// STD
#include <string>

namespace dds
{
    namespace topology_api
    {
        /// \class TopoProperty
        /// \brief Data class to hold topology property.
        class CTopoRequirement : public CTopoBase
        {
          public:
            enum class EType
            {
                WnName,
                HostName,
                Gpu
            };

            typedef std::shared_ptr<CTopoRequirement> Ptr_t;
            typedef std::vector<CTopoRequirement::Ptr_t> PtrVector_t;

          public:
            /// \brief Constructor.
            CTopoRequirement();

            /// \brief Destructor.
            virtual ~CTopoRequirement();

            /// \brief Inherited from TopoBase
            void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

            /// \brief Inherited from TopoBase
            void saveToPropertyTree(boost::property_tree::ptree& _pt);

            const std::string& getValue() const;
            CTopoRequirement::EType getRequirementType() const;

            void setValue(const std::string& _value);
            void setRequirementType(CTopoRequirement::EType _requireemntType);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoRequirement& _requirement);

          private:
            std::string m_value;                       ///< Requirement value
            CTopoRequirement::EType m_requirementType; ///< Requirement type
        };
    } // namespace topology_api
} // namespace dds

#endif /* defined(__DDS__TopoRequirement__) */
