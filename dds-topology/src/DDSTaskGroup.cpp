// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTaskGroup.h"
#include "DDSTopoFactory.h"
#include "DDSTopoUtils.h"
// STD
#include <memory>

using namespace std;
using namespace boost::property_tree;

DDSTaskGroup::DDSTaskGroup()
    : DDSTaskContainer()
    , m_n(0)
    , m_minimumRequired(0)
{
    setType(DDSTopoType::GROUP);
}

DDSTaskGroup::~DDSTaskGroup()
{
}

size_t DDSTaskGroup::getNofTasks() const
{
    return getNofTasksDefault();
}

size_t DDSTaskGroup::getTotalNofTasks() const
{
    const auto& elements = getElements();
    size_t counter = 0;
    for (const auto& v : elements)
    {
        counter += m_n * v->getTotalNofTasks();
    }
    return counter;
}

size_t DDSTaskGroup::getMinRequiredNofTasks() const
{
    const auto& elements = getElements();
    size_t counter = 0;
    for (const auto& v : elements)
    {
        counter += m_minimumRequired * v->getMinRequiredNofTasks();
    }
    return counter;
}

void DDSTaskGroup::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& mainPT = _pt.get_child("topology.main");
        const ptree& groupPT = (_name == "main") ? mainPT : DDSTopoElement::findElement(DDSTopoType::GROUP, _name, mainPT);

        setName(groupPT.get<string>("<xmlattr>.name"));
        setN(groupPT.get<size_t>("<xmlattr>.n"));
        setMinimumRequired(groupPT.get<size_t>("<xmlattr>.minRequired"));

        for (const auto& element : groupPT)
        {
            if (element.first == "<xmlattr>")
                continue;
            DDSTopoElementPtr_t newElement = DDSCreateTopoElement(DDSTagToTopoType(element.first));
            newElement->setParent(this);
            newElement->initFromPropertyTree(element.second.get<string>("<xmlattr>.name"), _pt);
            addElement(newElement);
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw runtime_error("Unable to initialize task group " + _name + " error: " + error.what());
    }
}

size_t DDSTaskGroup::getN() const
{
    return m_n;
}

size_t DDSTaskGroup::getMinimumRequired() const
{
    return m_minimumRequired;
}

void DDSTaskGroup::setN(size_t _n)
{
    m_n = _n;
}

void DDSTaskGroup::setMinimumRequired(size_t _minimumRequired)
{
    m_minimumRequired = _minimumRequired;
}

DDSTopoElementPtrVector_t DDSTaskGroup::getElementsByType(DDSTopoType _type) const
{
    const auto& elements = getElements();
    DDSTopoElementPtrVector_t result;
    for (const auto& v : elements)
    {
        if (v->getType() == _type)
        {
            result.push_back(v);
        }
        else if (v->getType() == DDSTopoType::GROUP)
        {
            DDSTopoElementPtrVector_t groupElements = dynamic_pointer_cast<DDSTaskGroup>(v)->getElementsByType(_type);
            result.insert(result.end(), groupElements.begin(), groupElements.end());
        }
    }
    return result;
}

DDSIndexVector_t DDSTaskGroup::getIndicesByType(DDSTopoType _type) const
{
    DDSIndexVector_t result;
    const auto& elements = getElements();
    for (const auto& v : elements)
    {
        if (v->getType() == _type)
        {
            result.push_back(v->getIndex());
        }
        else if (v->getType() == DDSTopoType::GROUP)
        {
            DDSTopoElementPtrVector_t groupElements = dynamic_pointer_cast<DDSTaskGroup>(v)->getElementsByType(_type);
            for (const auto& v : groupElements)
            {
                result.push_back(v->getIndex());
            }
        }
    }

    return result;
}

string DDSTaskGroup::toString() const
{
    stringstream ss;
    ss << "DDSTaskGroup: m_name=" << getName() << " m_n=" << m_n << " m_minimumRequired=" << m_minimumRequired << " nofElements=" << getNofElements()
       << " elements:\n";
    const auto& elements = getElements();
    for (const auto& element : elements)
    {
        ss << " - " << element->toString() << std::endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const DDSTaskGroup& _taskContainer)
{
    _strm << _taskContainer.toString();
    return _strm;
}
