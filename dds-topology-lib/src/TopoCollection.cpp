// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoCollection.h"
#include "TopoTask.h"
#include "TopoUtils.h"

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTopoCollection::CTopoCollection(const std::string& _name)
    : CTopoContainer(_name)
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

CTopoRequirement::Ptr_t CTopoCollection::addRequirement(const std::string& _name)
{
    auto requirement{ make_shared<CTopoRequirement>(_name) };
    requirement->setParent(this);
    m_requirements.push_back(requirement);
    return requirement;
}

void CTopoCollection::initFromPropertyTree(const ptree& _pt)
{
    try
    {
        const ptree& collectionPT =
            FindElementInPropertyTree(CTopoBase::EType::COLLECTION, getName(), _pt.get_child("topology"));

        boost::optional<const ptree&> requirementsPT = collectionPT.get_child_optional("requirements");
        if (requirementsPT)
        {
            for (const auto& requirement : requirementsPT.get())
            {
                addRequirement(requirement.second.data())->initFromPropertyTree(_pt);
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
                    addElement<CTopoTask>(data)->initFromPropertyTree(_pt);
                }
            }
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw runtime_error("Unable to initialize task collection " + getName() + " error: " + error.what());
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
