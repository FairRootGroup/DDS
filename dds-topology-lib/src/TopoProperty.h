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

namespace dds
{
    namespace topology_api
    {
        /// \class TopoProperty
        /// \brief Data class to hold topology property.
        class CTopoProperty : public CTopoBase
        {
          public:
            enum class EAccessType
            {
                READ,
                WRITE,
                READWRITE
            };

            enum class EScopeType
            {
                GLOBAL,
                COLLECTION
            };

            typedef std::shared_ptr<CTopoProperty> Ptr_t;
            // Property ID --> Ptr
            typedef std::map<std::string, CTopoProperty::Ptr_t> PtrMap_t;

          public:
            /// \brief Constructor.
            CTopoProperty();

            /// \brief Destructor.
            virtual ~CTopoProperty();

            /// \brief Inherited from TopoBase
            void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

            const std::string& getValue() const;
            void setValue(const std::string& _value);
            CTopoProperty::EAccessType getAccessType() const;
            void setAccessType(CTopoProperty::EAccessType _accessType);
            CTopoProperty::EScopeType getScopeType() const;
            void setScopeType(CTopoProperty::EScopeType _scopeType);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoProperty& _property);

          private:
            std::string m_value;                     ///< Property value
            CTopoProperty::EAccessType m_accessType; ///< Property access type
            CTopoProperty::EScopeType m_scopeType;   ///< Property scope type
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoProperty__) */
