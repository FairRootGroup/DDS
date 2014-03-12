// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTaskContainer__
#define __DDS__DDSTaskContainer__

// DDS
#include "DDSTopoElement.h"
// STD
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

class DDSTaskContainer : public DDSTopoElement
{
  public:
    size_t getNofElements() const;

    size_t getN() const;

    size_t getMinimumRequired() const;

    void setN(size_t _n);

    void setMinimumRequired(size_t _minimumRequired);

    /// \brief Return topology element by index.
    /// \return Topology element by index.
    /// \throw std::out_of_range
    DDSTopoElementPtr_t getElement(size_t _i) const;

    const DDSTopoElementPtrVector_t& getElements() const;

    void setElements(const DDSTopoElementPtrVector_t& _elements);

    void addElement(DDSTopoElementPtr_t _element);

    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const;

    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of
     * insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTaskContainer& _taskContainer);

  protected:
    /// \brief Constructor.
    DDSTaskContainer();

    /// \brief Destructor.
    virtual ~DDSTaskContainer();

    /// \brief Default implementation for DDSTopoElement::getNofTasks. Calculate recursively number of tasks in all daughter nodes.
    size_t getNofTasksDefault() const;

    /// \brief Default implementation for DDSTopoElement::initFromPropertyTree.
    void initFromPropertyTreeDefault(const std::string& _name, const boost::property_tree::ptree& _pt);

  private:
    DDSTopoElementPtrVector_t m_elements; ///> Vector of topology elements in collection.
    size_t m_n;                           ///> Number of times this task has to be executed
    size_t m_minimumRequired;             ///> Minimum required number of tasks to start processing
};

typedef std::shared_ptr<DDSTaskContainer> DDSTaskContainerPtr_t;
typedef std::vector<DDSTaskContainerPtr_t> DDSTaskContainerPtrVector_t;

#endif /* defined(__DDS__DDSTaskContainer__) */
