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
    virtual size_t getTotalNofTasks() const;

    /// \brief Inherited from DDSTopoElement.
    virtual size_t getMinRequiredNofTasks() const;

    /// \brief Inherited from DDSTopoElement.
    void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

    size_t getN() const;

    size_t getMinimumRequired() const;

    void setN(size_t _n);

    void setMinimumRequired(size_t _minimumRequired);

    /// \brief Returns string representation of an object.
    /// \return String representation of an object.
    virtual std::string toString() const;

    /// \brief Operator << for convenient output to ostream.
    /// \return Insertion stream in order to be able to call a succession of
    /// insertion operations.
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTaskGroup& _taskContainer);

  private:
    size_t m_n;               ///> Number of times this task has to be executed
    size_t m_minimumRequired; ///> Minimum required number of tasks to start processing
};

typedef std::shared_ptr<DDSTaskGroup> DDSTaskGroupPtr_t;
// typedef std::vector<DDSTaskGroupPtr_t> DDSTaskGroupPtrVector_t;

#endif
