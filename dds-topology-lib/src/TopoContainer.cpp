// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoContainer.h"
#include "TopoCollection.h"
#include "TopoGroup.h"
#include "TopoTask.h"
// STD
#include <memory>
#include <sstream>
#include <string>
// BOOST
#include <boost/property_tree/ptree.hpp>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTopoContainer::CTopoContainer()
    : CTopoElement()
    , m_elements()
{
}

CTopoContainer::~CTopoContainer()
{
}

size_t CTopoContainer::getNofElements() const
{
    return m_elements.size();
}

CTopoElement::Ptr_t CTopoContainer::getElement(size_t _i) const
{
    if (_i >= getNofElements())
        throw std::out_of_range("Out of range exception");
    return m_elements[_i];
}

const CTopoElement::PtrVector_t& CTopoContainer::getElements() const
{
    return m_elements;
}

void CTopoContainer::setElements(const CTopoElement::PtrVector_t& _elements)
{
    m_elements = _elements;
}

void CTopoContainer::addElement(CTopoElement::Ptr_t _element)
{
    m_elements.push_back(_element);
}

size_t CTopoContainer::getNofTasksDefault() const
{
    const auto& elements = getElements();
    size_t counter = 0;
    for (const auto& v : elements)
    {
        counter += v->getNofTasks();
    }
    return counter;
}

string CTopoContainer::toString() const
{
    stringstream ss;
    ss << "TaskContainer: m_name=" << getName() << " nofElements=" << getNofElements() << " elements:\n";
    for (const auto& element : m_elements)
    {
        ss << " - " << element->toString() << std::endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoContainer& _taskContainer)
{
    _strm << _taskContainer.toString();
    return _strm;
}
