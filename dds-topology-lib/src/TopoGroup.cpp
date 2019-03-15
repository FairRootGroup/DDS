// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoGroup.h"
#include "TopoFactory.h"
#include "TopoUtils.h"
// STD
#include <memory>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTopoGroup::CTopoGroup()
    : CTopoContainer()
    , m_n(1)
{
    setType(CTopoBase::EType::GROUP);
}

CTopoGroup::~CTopoGroup()
{
}

size_t CTopoGroup::getNofTasks() const
{
    return getNofTasksDefault();
}

size_t CTopoGroup::getTotalNofTasks() const
{
    const auto& elements = getElements();
    size_t counter = 0;
    for (const auto& v : elements)
    {
        counter += m_n * v->getTotalNofTasks();
    }
    return counter;
}

void CTopoGroup::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& mainPT = _pt.get_child("topology.main");
        const ptree& groupPT =
            (_name == "main") ? mainPT : CTopoElement::findElement(CTopoBase::EType::GROUP, _name, mainPT);

        setName(groupPT.get<string>("<xmlattr>.name"));
        setN(groupPT.get<size_t>("<xmlattr>.n", 1));

        for (const auto& element : groupPT)
        {
            if (element.first == "<xmlattr>")
                continue;
            CTopoElement::Ptr_t newElement = CreateTopoElement(UseTagToTopoType(element.first));
            newElement->setParent(this);
            boost::optional<const ptree&> child = element.second.get_child_optional("<xmlattr>");
            string name = (child) ? child.get().get<string>("name") : element.second.data();
            newElement->initFromPropertyTree(name, _pt);
            addElement(newElement);
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw runtime_error("Unable to initialize task group " + _name + " error: " + error.what());
    }
}

size_t CTopoGroup::getN() const
{
    return m_n;
}

void CTopoGroup::setN(size_t _n)
{
    m_n = _n;
}

CTopoElement::PtrVector_t CTopoGroup::getElementsByType(CTopoBase::EType _type) const
{
    const auto& elements = getElements();
    CTopoElement::PtrVector_t result;
    for (const auto& v : elements)
    {
        if (v->getType() == _type)
        {
            result.push_back(v);
        }
        else if (v->getType() == CTopoBase::EType::GROUP)
        {
            CTopoElement::PtrVector_t groupElements = dynamic_pointer_cast<CTopoGroup>(v)->getElementsByType(_type);
            result.insert(result.end(), groupElements.begin(), groupElements.end());
        }
    }
    return result;
}

string CTopoGroup::toString() const
{
    stringstream ss;
    ss << "TaskGroup: m_name=" << getName() << " m_n=" << m_n << " nofElements=" << getNofElements() << " elements:\n";
    const auto& elements = getElements();
    for (const auto& element : elements)
    {
        ss << " - " << element->toString() << std::endl;
    }
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoGroup& _taskContainer)
{
    _strm << _taskContainer.toString();
    return _strm;
}
