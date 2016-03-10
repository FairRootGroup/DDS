// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__Requirement__
#define __DDS__Requirement__

// DDS
#include "TopoBase.h"
// STD
#include <string>

namespace dds
{
    namespace topology_api
    {
        enum class EHostPatternType
        {
            WnName,
            HostName
        };

        /// \class TopoProperty
        /// \brief Data class to hold topology property.
        class CRequirement : public CTopoBase
        {
          public:
            /// \brief Constructor.
            CRequirement();

            /// \brief Destructor.
            virtual ~CRequirement();

            /// \brief Inherited from TopoBase
            void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

            const std::string& getHostPattern() const;
            EHostPatternType getHostPatternType() const;

            void setHostPattern(const std::string& _hostPattern);
            void setHostPatternType(EHostPatternType _hostPatternType);

            bool hostPatterMatches(const std::string& _host) const;

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CRequirement& _requirement);

          private:
            std::string m_hostPattern;          ///< Pattern of the host name
            EHostPatternType m_hostPatternType; ///< Type of the host pattern
        };

        typedef std::shared_ptr<CRequirement> RequirementPtr_t;
        typedef std::vector<RequirementPtr_t> RequirementPtrVector_t;
    }
}

#endif /* defined(__DDS__Requirement__) */
