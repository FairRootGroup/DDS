// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTaskCollection__
#define __DDS__DDSTaskCollection__

// DDS
#include "DDSTaskContainer.h"

class DDSTaskCollection : public DDSTaskContainer
{
  public:
    /// \brief Constructor.
    DDSTaskCollection();

    /// \brief Destructor.
    virtual ~DDSTaskCollection();

    /// \brief Inherited from DDSTopoElement.
    virtual size_t getNofTasks() const;
    
    /// \brief Inherited from DDSTopoElement.
    virtual size_t getTotalNofTasks() const;
    
    /// \brief Inherited from DDSTopoElement.
    virtual size_t getMinRequiredNofTasks() const;

    /// \brief Inherited from DDSTopoElement.
    void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

  private:
};

typedef std::shared_ptr<DDSTaskCollection> DDSTaskCollectionPtr_t;
// typedef std::vector<DDSTaskCollectionPtr_t> DDSTaskCollectionPtrVector_t;

#endif /* defined(__DDS__DDSTopology__) */
