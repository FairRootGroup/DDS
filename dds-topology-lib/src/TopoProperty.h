// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoProperty__
#define __DDS__TopoProperty__

// DDS
#include "TopoBase.h"
// STD
#include <sstream>
#include <string>
#include <vector>
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
            EPropertyAccessType getAccessType() const;

            void setValue(const std::string& _value);
            void setAccessType(EPropertyAccessType _accessType);

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
        };

        typedef std::shared_ptr<CTopoProperty> TopoPropertyPtr_t;
        typedef std::vector<TopoPropertyPtr_t> TopoPropertyPtrVector_t;
    }
}
#endif /* defined(__DDS__TopoProperty__) */
