// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopoProperty__
#define __DDS__DDSTopoProperty__

// DDS
#include "DDSTopoBase.h"
// STD
#include <sstream>
#include <string>
#include <vector>
// BOOST
#include <boost/property_tree/ptree.hpp>

class DDSTopoProperty : public DDSTopoBase
{
  protected:
    /// \brief Constructor.
    DDSTopoProperty();

    /// \brief Destructor.
    virtual ~DDSTopoProperty();
};

typedef std::shared_ptr<DDSTopoProperty> DDSTopoPropertyPtr_t;
typedef std::vector<DDSTopoPropertyPtr_t> DDSTopoPropertyPtrVector_t;

#endif /* defined(__DDS__DDSTopoProperty__) */
