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
    , m_n(0)
    , m_minimumRequired(0)
{
}

DDSTaskContainer::~DDSTaskContainer()
{
}

size_t DDSTaskContainer::getNofElements() const
{
    return m_elements.size();
}

size_t DDSTaskContainer::getN() const
{
    return m_n;
}

size_t DDSTaskContainer::getMinimumRequired() const
{
    return m_minimumRequired;
}

void DDSTaskContainer::setN(size_t _n)
{
    m_n = _n;
}

void DDSTaskContainer::setMinimumRequired(size_t _minimumRequired)
{
    m_minimumRequired = _minimumRequired;
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

void DDSTaskContainer::initFromPropertyTreeDefault(const string& _name, const ptree& _pt)
{
    bool initialized = false;
    try
    {
        const ptree& topPT = _pt.get_child("topology");
        for (const auto& v : topPT)
        {
            if (!(v.first == "collection" || v.first == "group"))
                continue;

            const auto& containerPT = v.second;
            string containerName = containerPT.get<string>("<xmlattr>.name");
            if (containerName == _name)
            {
                setName(containerName);

                for (const auto& element : containerPT)
                {
                    if (element.first == "task")
                    {
                        DDSTaskPtr_t newTask = make_shared<DDSTask>();
                        string taskName = element.second.get<string>("<xmlattr>.name");
                        newTask->initFromPropertyTree(taskName, _pt);
                        addElement(newTask);
                    }
                    else if (element.first == "collection")
                    {
                        DDSTaskCollectionPtr_t newCollection = make_shared<DDSTaskCollection>();
                        string collectionName = element.second.get<string>("<xmlattr>.name");
                        newCollection->initFromPropertyTree(collectionName, _pt);
                        addElement(newCollection);
                    }
                }

                initialized = true;
                break;
            }
        }
    }
    catch (ptree_error& error)
    {
        cout << "ptree_error: " << error.what() << endl;
    }

    if (!initialized)
        throw logic_error("Unable to initialize task container " + _name);
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
    ss << "DDSTaskContainer: m_name=" << getName() << " m_n=" << m_n << " m_minimumRequired=" << m_minimumRequired << " nofElements=" << getNofElements()
       << " elements:\n";
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
