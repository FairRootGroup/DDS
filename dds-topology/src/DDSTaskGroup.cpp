// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "DDSTaskGroup.h"
#include "DDSTask.h"
#include "DDSTaskCollection.h"

using namespace std;
using namespace boost::property_tree;

DDSTaskGroup::DDSTaskGroup()
    : DDSTaskContainer()
    , m_n(0)
    , m_minimumRequired(0)
{
    setType(DDSTopoElementType::GROUP);
}

DDSTaskGroup::~DDSTaskGroup()
{
}

size_t DDSTaskGroup::getNofTasks() const
{
    return getNofTasksDefault();
}

void DDSTaskGroup::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& groupPT = (_name != "main") ? DDSTopoElement::findElement("group", _name, _pt) : DDSTopoElement::findElement("main", _name, _pt);

        setName(groupPT.get<string>("<xmlattr>.name"));
        setN(groupPT.get<size_t>("<xmlattr>.n"));
        setMinimumRequired(groupPT.get<size_t>("<xmlattr>.minRequired"));

        for (const auto& element : groupPT)
        {
            if (element.first == "task" || element.first == "collection" || element.first == "group")
            {
                DDSTopoElementPtr_t newElement = nullptr;
                if (element.first == "task")
                    newElement = dynamic_pointer_cast<DDSTopoElement>(make_shared<DDSTask>());
                else if (element.first == "collection")
                    newElement = dynamic_pointer_cast<DDSTopoElement>(make_shared<DDSTaskCollection>());
                else if (element.first == "group")
                    newElement = dynamic_pointer_cast<DDSTopoElement>(make_shared<DDSTaskGroup>());

                newElement->initFromPropertyTree(element.second.get<string>("<xmlattr>.name"), _pt);
                addElement(newElement);
            }
        }
    }
    catch (ptree_error& error)
    {
        throw logic_error("Unable to initialize task group " + _name + " error: " + error.what());
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
