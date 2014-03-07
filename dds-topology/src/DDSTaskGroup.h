// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_DDSTaskGroup_h
#define DDS_DDSTaskGroup_h
// DDS
#include "DDSTaskContainer.h"
// STD
#include <ostream>
#include <sstream>
#include <vector>
#include <memory>

class DDSTaskGroup : public DDSTaskContainer
{
public:
    /**
     * \brief Constructor.
     */
    DDSTaskGroup() : DDSTaskContainer()
    {
        setType(DDSTopoElementType::GROUP);
    }

    /**
     * \brief Destructor.
     */
    virtual ~DDSTaskGroup()
    {
    }

    virtual size_t getNofTasks() const
    {
        return getNofTasksDefault();
    }

private:
};

typedef std::shared_ptr<DDSTaskGroup> DDSTaskGroupPtr_t;
// typedef std::vector<DDSTaskGroupPtr_t> DDSTaskGroupPtrVector_t;

#endif
