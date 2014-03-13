// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTaskContainer.h"
#include "DDSTaskGroup.h"
#include "DDSTaskCollection.h"
#include "DDSTask.h"
// STD
#include <memory>
#include <sstream>
#include <string>
// BOOST
#include <boost/property_tree/ptree.hpp>

using namespace std;
using namespace boost::property_tree;

DDSTaskContainer::DDSTaskContainer()
    : DDSTopoElement()
    , m_elements()
{
}

DDSTaskContainer::~DDSTaskContainer()
{
}

size_t DDSTaskContainer::getNofElements() const
{
    return m_elements.size();
}

DDSTopoElementPtr_t DDSTaskContainer::getElement(size_t _i) const
{
    if (_i >= getNofElements())
        throw std::out_of_range("Out of range exception");
    return m_elements[_i];
}

const DDSTopoElementPtrVector_t& DDSTaskContainer::getElements() const
{
    return m_elements;
}

void DDSTaskContainer::setElements(const DDSTopoElementPtrVector_t& _elements)
{
    m_elements = _elements;
}

void DDSTaskContainer::addElement(DDSTopoElementPtr_t _element)
{
    m_elements.push_back(_element);
}

size_t DDSTaskContainer::getNofTasksDefault() const
{
    const auto& elements = getElements();
    size_t counter = 0;
    for (const auto& v : elements)
    {
        counter += v->getNofTasks();
    }
    return counter;
}

string DDSTaskContainer::toString() const
{
    stringstream ss;
    ss << "DDSTaskContainer: m_name=" << getName() << " nofElements=" << getNofElements() << " elements:\n";
    for (const auto& element : m_elements)
    {
        ss << " - " << element->toString() << std::endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const DDSTaskContainer& _taskContainer)
{
    _strm << _taskContainer.toString();
    return _strm;
}
