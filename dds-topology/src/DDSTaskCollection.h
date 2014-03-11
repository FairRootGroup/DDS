// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTaskCollection__
#define __DDS__DDSTaskCollection__

// DDS
#include "DDSTaskContainer.h"
// STD
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

class DDSTaskCollection : public DDSTaskContainer
{
  public:
    /**
     * \brief Constructor.
     */
    DDSTaskCollection()
        : DDSTaskContainer()
    {
        setType(DDSTopoElementType::COLLECTION);
    }

    /**
     * \brief Destructor.
     */
    virtual ~DDSTaskCollection()
    {
    }

    virtual size_t getNofTasks() const
    {
        return getNofTasksDefault();
    }

    //    /**
    //     * \brief Returns string representation of an object.
    //     * \return String representation of an object.
    //     */
    //    std::string toString() const
    //    {
    //        std::stringstream ss;
    //        ss << "DDSTaskCollection: m_name=" << getName() << " nofTasks=" << m_tasks.size() << " tasks:";
    //        for (const auto& task : m_tasks)
    //        {
    //            ss << " - " << task->toString() << std::endl;
    //        }
    //        return ss.str();
    //    }
    //
    //    /**
    //     * \brief Operator << for convenient output to ostream.
    //     * \return Insertion stream in order to be able to call a succession of
    //     * insertion operations.
    //     */
    //    friend std::ostream& operator<<(std::ostream& _strm, const DDSTaskCollection& _taskCollection)
    //    {
    //        _strm << _taskCollection.toString();
    //        return _strm;
    //    }

  private:
};

typedef std::shared_ptr<DDSTaskCollection> DDSTaskCollectionPtr_t;
// typedef std::vector<DDSTaskCollectionPtr_t> DDSTaskCollectionPtrVector_t;

#endif /* defined(__DDS__DDSTopology__) */
