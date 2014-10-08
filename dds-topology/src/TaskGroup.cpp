// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TaskGroup.h"
#include "TopoFactory.h"
#include "TopoUtils.h"
// STD
#include <memory>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

CTaskGroup::CTaskGroup()
    : CTaskContainer()
    , m_n(1)
{
    setType(ETopoType::GROUP);
}

CTaskGroup::~CTaskGroup()
{
}

size_t CTaskGroup::getNofTasks() const
{
    return getNofTasksDefault();
}

size_t CTaskGroup::getTotalNofTasks() const
{
    const auto& elements = getElements();
    size_t counter = 0;
    for (const auto& v : elements)
    {
        counter += m_n * v->getTotalNofTasks();
    }
    return counter;
}

void CTaskGroup::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& mainPT = _pt.get_child("topology.main");
        const ptree& groupPT = (_name == "main") ? mainPT : CTopoElement::findElement(ETopoType::GROUP, _name, mainPT);

        setId(groupPT.get<string>("<xmlattr>.id"));
        setN(groupPT.get<size_t>("<xmlattr>.n", 1));

        for (const auto& element : groupPT)
        {
            if (element.first == "<xmlattr>")
                continue;
            TopoElementPtr_t newElement = CreateTopoElement(UseTagToTopoType(element.first));
            newElement->setParent(this);
            boost::optional<const ptree&> child = element.second.get_child_optional("<xmlattr>");
            string name = (child) ? child.get().get<string>("id") : element.second.data();
            newElement->initFromPropertyTree(name, _pt);
            addElement(newElement);
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw runtime_error("Unable to initialize task group " + _name + " error: " + error.what());
    }
}

size_t CTaskGroup::getN() const
{
    return m_n;
}

void CTaskGroup::setN(size_t _n)
{
    m_n = _n;
}

TopoElementPtrVector_t CTaskGroup::getElementsByType(ETopoType _type) const
{
    const auto& elements = getElements();
    TopoElementPtrVector_t result;
    for (const auto& v : elements)
    {
        if (v->getType() == _type)
        {
            result.push_back(v);
        }
        else if (v->getType() == ETopoType::GROUP)
        {
            TopoElementPtrVector_t groupElements = dynamic_pointer_cast<CTaskGroup>(v)->getElementsByType(_type);
            result.insert(result.end(), groupElements.begin(), groupElements.end());
        }
    }
    return result;
}

IndexVector_t CTaskGroup::getIndicesByType(ETopoType _type) const
{
    IndexVector_t result;
    const auto& elements = getElements();
    for (const auto& v : elements)
    {
        if (v->getType() == _type)
        {
            result.push_back(v->getIndex());
        }
        else if (v->getType() == ETopoType::GROUP)
        {
            TopoElementPtrVector_t groupElements = dynamic_pointer_cast<CTaskGroup>(v)->getElementsByType(_type);
            for (const auto& v : groupElements)
            {
                result.push_back(v->getIndex());
            }
        }
    }

    return result;
}

string CTaskGroup::toString() const
{
    stringstream ss;
    ss << "TaskGroup: m_id=" << getId() << " m_n=" << m_n << " nofElements=" << getNofElements() << " elements:\n";
    const auto& elements = getElements();
    for (const auto& element : elements)
    {
        ss << " - " << element->toString() << std::endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTaskGroup& _taskContainer)
{
    _strm << _taskContainer.toString();
    return _strm;
}
