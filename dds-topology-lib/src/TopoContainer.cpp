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

CTopoContainer::CTopoContainer(const std::string& _name)
    : CTopoElement(_name)
{
    if (_name == "main")
    {
        setNameToValueCache(make_shared<CTopoBase::CNameToValueCachePtr_t::element_type>());
    }
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

template <>
CTopoTask::Ptr_t CTopoContainer::addElement<CTopoTask>(const std::string& _name)
{
    if (!canAddElement(CTopoBase::EType::TASK))
        throw runtime_error("Can't add task to TopoContainer");
    return makeElement<CTopoTask>(_name);
}

template <>
CTopoCollection::Ptr_t CTopoContainer::addElement<CTopoCollection>(const std::string& _name)
{
    if (!canAddElement(CTopoBase::EType::COLLECTION))
        throw runtime_error("Can't add collection to TopoContainer");
    return makeElement<CTopoCollection>(_name);
}

template <>
CTopoGroup::Ptr_t CTopoContainer::addElement<CTopoGroup>(const std::string& _name)
{
    if (!canAddElement(CTopoBase::EType::GROUP))
        throw runtime_error("Can't add group to TopoContainer");
    return makeElement<CTopoGroup>(_name);
}

CTopoElement::Ptr_t CTopoContainer::addElement(CTopoBase::EType _type, const std::string& _name)
{
    switch (_type)
    {
        case CTopoBase::EType::TASK:
            return addElement<CTopoTask>(_name);
        case CTopoBase::EType::COLLECTION:
            return addElement<CTopoCollection>(_name);
        case CTopoBase::EType::GROUP:
            return addElement<CTopoGroup>(_name);
        default:
            throw runtime_error("Specified type is not a TopoElement.");
    }
}

bool CTopoContainer::canAddElement(CTopoBase::EType _type)
{
    switch (_type)
    {
        case CTopoBase::EType::TASK:
            return true;

        case CTopoBase::EType::COLLECTION:
            return getType() == CTopoBase::EType::GROUP;

        case CTopoBase::EType::GROUP:
            return (getType() == CTopoBase::EType::GROUP && getParent() == nullptr);

        default:
            return false;
    }
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
