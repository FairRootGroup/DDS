// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_DDSTaskGroup_h
#define DDS_DDSTaskGroup_h
// DDS
#include "DDSTaskContainer.h"

class DDSTaskGroup : public DDSTaskContainer
{
  public:
    /// \brief Constructor.
    DDSTaskGroup();

    /// \brief Destructor.
    virtual ~DDSTaskGroup();

    /// \brief Inherited from DDSTopoElement.
    virtual size_t getNofTasks() const;

    /// \brief Inherited from DDSTopoElement.
    void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

  private:
};

typedef std::shared_ptr<DDSTaskGroup> DDSTaskGroupPtr_t;
// typedef std::vector<DDSTaskGroupPtr_t> DDSTaskGroupPtrVector_t;

#endif
