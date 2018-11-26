// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoProperty__
#define __DDS__TopoProperty__

// DDS
#include "TopoBase.h"
// STD
#include <map>
#include <sstream>
#include <string>
// BOOST
#include <boost/property_tree/ptree.hpp>

namespace dds
{
    namespace topology_api
    {
        enum class EPropertyAccessType
        {
            READ,
            WRITE,
            READWRITE
        };

        enum class EPropertyScopeType
        {
            GLOBAL,
            COLLECTION
        };

        /// \class TopoProperty
        /// \brief Data class to hold topology property.
        class CTopoProperty : public CTopoBase
        {
          public:
            /// \brief Constructor.
            CTopoProperty();

            /// \brief Destructor.
            virtual ~CTopoProperty();

            /// \brief Inherited from TopoBase
            void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

            const std::string& getValue() const;
            void setValue(const std::string& _value);
            EPropertyAccessType getAccessType() const;
            void setAccessType(EPropertyAccessType _accessType);
            EPropertyScopeType getScopeType() const;
            void setScopeType(EPropertyScopeType _scopeType);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoProperty& _property);

          private:
            std::string m_value;              ///< Property value
            EPropertyAccessType m_accessType; ///< Property access type
            EPropertyScopeType m_scopeType;   ///< Property scope type
        };

        typedef std::shared_ptr<CTopoProperty> TopoPropertyPtr_t;
        // Property ID --> Ptr
        typedef std::map<std::string, TopoPropertyPtr_t> TopoPropertyPtrMap_t;
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoProperty__) */
