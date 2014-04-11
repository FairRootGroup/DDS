// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_DDSTaskGroup_h
#define DDS_DDSTaskGroup_h
// DDS
#include "DDSTaskContainer.h"
#include "DDSIndex.h"

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

    /// \brief Inherited from DDSTopoBase.
    void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

    size_t getN() const;

    void setN(size_t _n);

    DDSTopoElementPtrVector_t getElementsByType(DDSTopoType _type) const;

    DDSIndexVector_t getIndicesByType(DDSTopoType _type) const;

    /// \brief Returns string representation of an object.
    /// \return String representation of an object.
    virtual std::string toString() const;

    /// \brief Operator << for convenient output to ostream.
    /// \return Insertion stream in order to be able to call a succession of
    /// insertion operations.
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTaskGroup& _taskContainer);

  private:
    size_t m_n; ///> Number of times this group has to be executed
};

typedef std::shared_ptr<DDSTaskGroup> DDSTaskGroupPtr_t;
// typedef std::vector<DDSTaskGroupPtr_t> DDSTaskGroupPtrVector_t;

#endif
