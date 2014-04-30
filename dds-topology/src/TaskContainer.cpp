// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TaskContainer.h"
#include "TaskGroup.h"
#include "TaskCollection.h"
#include "Task.h"
// STD
#include <memory>
#include <sstream>
#include <string>
// BOOST
#include <boost/property_tree/ptree.hpp>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

CTaskContainer::CTaskContainer()
    : CTopoElement()
    , m_elements()
{
}

CTaskContainer::~CTaskContainer()
{
}

size_t CTaskContainer::getNofElements() const
{
    return m_elements.size();
}

TopoElementPtr_t CTaskContainer::getElement(size_t _i) const
{
    if (_i >= getNofElements())
        throw std::out_of_range("Out of range exception");
    return m_elements[_i];
}

const TopoElementPtrVector_t& CTaskContainer::getElements() const
{
    return m_elements;
}

void CTaskContainer::setElements(const TopoElementPtrVector_t& _elements)
{
    m_elements = _elements;
}

void CTaskContainer::addElement(TopoElementPtr_t _element)
{
    m_elements.push_back(_element);
}

size_t CTaskContainer::getNofTasksDefault() const
{
    const auto& elements = getElements();
    size_t counter = 0;
    for (const auto& v : elements)
    {
        counter += v->getNofTasks();
    }
    return counter;
}

string CTaskContainer::toString() const
{
    stringstream ss;
    ss << "TaskContainer: m_name=" << getName() << " nofElements=" << getNofElements() << " elements:\n";
    for (const auto& element : m_elements)
    {
        ss << " - " << element->toString() << std::endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTaskContainer& _taskContainer)
{
    _strm << _taskContainer.toString();
    return _strm;
}
