// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoCollection.h"
#include "TopoFactory.h"
#include "TopoTask.h"
#include "TopoUtils.h"

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTopoCollection::CTopoCollection()
    : CTopoContainer()
{
    setType(CTopoBase::EType::COLLECTION);
}

CTopoCollection::~CTopoCollection()
{
}

size_t CTopoCollection::getNofTasks() const
{
    return getNofTasksDefault();
}

size_t CTopoCollection::getTotalNofTasks() const
{
    return getNofTasksDefault();
}

size_t CTopoCollection::getTotalCounter() const
{
    return getTotalCounterDefault();
}

size_t CTopoCollection::getNofRequirements() const
{
    return m_requirements.size();
}

const CTopoRequirement::PtrVector_t& CTopoCollection::getRequirements() const
{
    return m_requirements;
}

void CTopoCollection::setRequirement(const CTopoRequirement::PtrVector_t& _requirements)
{
    m_requirements = _requirements;
}

void CTopoCollection::addRequirement(CTopoRequirement::Ptr_t _requirement)
{
    m_requirements.push_back(_requirement);
}

void CTopoCollection::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& collectionPT =
            FindElementInPropertyTree(CTopoBase::EType::COLLECTION, _name, _pt.get_child("topology"));

        setName(collectionPT.get<string>("<xmlattr>.name"));

        boost::optional<const ptree&> requirementsPT = collectionPT.get_child_optional("requirements");
        if (requirementsPT)
        {
            for (const auto& requirement : requirementsPT.get())
            {
                CTopoRequirement::Ptr_t newRequirement = make_shared<CTopoRequirement>();
                newRequirement->setParent(this);
                newRequirement->initFromPropertyTree(requirement.second.data(), _pt);
                addRequirement(newRequirement);
            }
        }

        boost::optional<const ptree&> tasksPT = collectionPT.get_child_optional("tasks");
        if (tasksPT)
        {
            for (const auto& task : tasksPT.get())
            {
                const size_t n{ task.second.get<size_t>("<xmlattr>.n", 1) };
                const string data{ task.second.data() };
                for (size_t i = 0; i < n; i++)
                {
                    CTopoElement::Ptr_t newElement = make_shared<CTopoTask>();
                    newElement->setParent(this);
                    newElement->initFromPropertyTree(data, _pt);
                    addElement(newElement);
                }
            }
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw runtime_error("Unable to initialize task collection " + _name + " error: " + error.what());
    }
}

void CTopoCollection::saveToPropertyTree(boost::property_tree::ptree& _pt)
{
    try
    {
        std::string tag("topology.declcollection");
        _pt.put(tag + ".<xmlattr>.name", getName());

        for (const auto& v : m_requirements)
        {
            _pt.add(tag + ".requirements.name", v->getName());
        }

        for (const auto& v : getElements())
        {
            _pt.add(tag + ".tasks.name", v->getName());
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw runtime_error("Unable to save task collection" + getName() + " error: " + error.what());
    }
}
