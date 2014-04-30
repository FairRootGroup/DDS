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
    /// \class TopoProperty
    /// \brief Data class to hold topology property.
    class CTopoProperty : public CTopoBase
    {
      protected:
        /// \brief Constructor.
        CTopoProperty();

        /// \brief Destructor.
        virtual ~CTopoProperty();
    };

    typedef std::shared_ptr<CTopoProperty> TopoPropertyPtr_t;
    typedef std::vector<TopoPropertyPtr_t> TopoPropertyPtrVector_t;
}
#endif /* defined(__DDS__TopoProperty__) */
