// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopoElement__
#define __DDS__DDSTopoElement__

// DDS
#include "DDSTopoBase.h"
// STD
#include <sstream>
#include <string>
#include <vector>
// BOOST
#include <boost/property_tree/ptree.hpp>

class DDSTopoElement : public DDSTopoBase
{
  public:
    /// \brief Has to return number of all tasks including daughter elements.
    virtual size_t getNofTasks() const = 0;

  protected:
    /// \brief Constructor.
    DDSTopoElement();

    /// \brief Destructor.
    virtual ~DDSTopoElement();
};

typedef std::shared_ptr<DDSTopoElement> DDSTopoElementPtr_t;
typedef std::vector<DDSTopoElementPtr_t> DDSTopoElementPtrVector_t;

#endif /* defined(__DDS__DDSTopoElement__) */
