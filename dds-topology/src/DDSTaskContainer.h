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
#include <string>
#include <vector>

class DDSTaskContainer : public DDSTopoElement
{
  public:
    /// \brief Return number of elements.
    /// \return Number of elements.
    size_t getNofElements() const;

    /// \brief Return topology element by index.
    /// \return Topology element by index.
    /// \throw std::out_of_range
    DDSTopoElementPtr_t getElement(size_t _i) const;

    /// \brief Return vector of elements.
    /// \return Vector of elements.
    const DDSTopoElementPtrVector_t& getElements() const;

    void setElements(const DDSTopoElementPtrVector_t& _elements);

    void addElement(DDSTopoElementPtr_t _element);

    /// \brief Returns string representation of an object.
    /// \return String representation of an object.
    virtual std::string toString() const;

    /// \brief Operator << for convenient output to ostream.
    /// \return Insertion stream in order to be able to call a succession of
    /// insertion operations.
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTaskContainer& _taskContainer);

  protected:
    /// \brief Constructor.
    DDSTaskContainer();

    /// \brief Destructor.
    virtual ~DDSTaskContainer();

    /// \brief Default implementation for DDSTopoElement::getNofTasks. Calculate recursively number of tasks in all daughter elements.
    size_t getNofTasksDefault() const;

  private:
    DDSTopoElementPtrVector_t m_elements; ///> Vector of topology elements in collection.
};

typedef std::shared_ptr<DDSTaskContainer> DDSTaskContainerPtr_t;
typedef std::vector<DDSTaskContainerPtr_t> DDSTaskContainerPtrVector_t;

#endif /* defined(__DDS__DDSTaskContainer__) */
