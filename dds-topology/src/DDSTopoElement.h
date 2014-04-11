// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopoElement__
#define __DDS__DDSTopoElement__

// DDS
#include "DDSTopoBase.h"
// STD
#include <string>

class DDSTopoElement : public DDSTopoBase
{
  public:
    /// \brief Return number of all tasks including daughter elements.
    virtual size_t getNofTasks() const = 0;

    /// \brief Return total number of tasks, i.e. number of tasks multiplied by n.
    virtual size_t getTotalNofTasks() const = 0;

  protected:
    /// \brief Constructor.
    DDSTopoElement();

    /// \brief Destructor.
    virtual ~DDSTopoElement();

    /// \brief If parent is a group than return N, else return 1.
    /// Default implementation for DDSTask::getTotalCounter and DDSTaskCollection::getTotalCounter.
    size_t getTotalCounterDefault() const;

  private:
    std::string m_path; // Full path to element, including element name
};

typedef std::shared_ptr<DDSTopoElement> DDSTopoElementPtr_t;
typedef std::vector<DDSTopoElementPtr_t> DDSTopoElementPtrVector_t;

#endif /* defined(__DDS__DDSTopoElement__) */
