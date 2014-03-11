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
    size_t getNofElements() const
    {
        return m_elements.size();
    }

    size_t getN() const
    {
        return m_n;
    }

    size_t getMinimumRequired() const
    {
        return m_minimumRequired;
    }

    void setN(size_t _n)
    {
        m_n = _n;
    }

    void setMinimumRequired(size_t _minimumRequired)
    {
        m_minimumRequired = _minimumRequired;
    }

    /**
     * \brief Return topology element by index.
     * \return Topology element by index.
     * \throw std::out_of_range
     */
    DDSTopoElementPtr_t getElement(size_t _i) const
    {
        if (_i >= getNofElements())
            throw std::out_of_range("Out of range exception");
        return m_elements[_i];
    }

    const DDSTopoElementPtrVector_t& getElements() const
    {
        return m_elements;
    }

    void setElements(const DDSTopoElementPtrVector_t& _elements);

    void addElement(DDSTopoElementPtr_t _element);

    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const
    {
        std::stringstream ss;
        ss << "DDSTaskContainer: m_name=" << getName() << " m_n=" << m_n << " m_minimumRequired=" << m_minimumRequired << " nofElements=" << getNofElements()
           << " elements:\n";
        for (const auto& element : m_elements)
        {
            ss << " - " << element->toString() << std::endl;
        }
        return ss.str();
    }

    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of
     * insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTaskContainer& _taskContainer)
    {
        _strm << _taskContainer.toString();
        return _strm;
    }

  protected:
    /**
     * \brief Constructor.
     */
    DDSTaskContainer()
        : DDSTopoElement()
        , m_elements()
        , m_n(0)
        , m_minimumRequired(0)
    {
    }

    /**
     * \brief Destructor.
     */
    virtual ~DDSTaskContainer()
    {
    }

    /**
     * \brief Copy Constructor
     **/
    DDSTaskContainer(const DDSTaskContainer&);

    /**
     * \brief Assignment Operator
     **/
    DDSTaskContainer& operator=(const DDSTaskContainer&);

    size_t getNofTasksDefault() const
    {
        const auto& elements = getElements();
        size_t counter = 0;
        for (const auto& v : elements)
        {
            counter += v->getNofTasks();
        }
        return counter;
    }

  private:
    /**
     * \brief Make a deep copy of the object. Used in copy constructor and assignment operator.
     */
    void deepCopy(const DDSTaskContainer&);

    DDSTopoElementPtrVector_t m_elements; ///> Vector of topology elements in collection.
    size_t m_n;                           ///> Number of times this task has to be executed
    size_t m_minimumRequired;             ///> Minimum required number of tasks to start processing
};

typedef std::shared_ptr<DDSTaskContainer> DDSTaskContainerPtr_t;
typedef std::vector<DDSTaskContainerPtr_t> DDSTaskContainerPtrVector_t;

#endif /* defined(__DDS__DDSTaskContainer__) */
