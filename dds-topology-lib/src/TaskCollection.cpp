// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TaskCollection.h"
#include "Task.h"
#include "TopoFactory.h"
#include "TopoUtils.h"

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTaskCollection::CTaskCollection()
    : CTaskContainer()
{
    setType(ETopoType::COLLECTION);
}

CTaskCollection::~CTaskCollection()
{
}

size_t CTaskCollection::getNofTasks() const
{
    return getNofTasksDefault();
}

size_t CTaskCollection::getTotalNofTasks() const
{
    return getNofTasksDefault();
}

size_t CTaskCollection::getTotalCounter() const
{
    return getTotalCounterDefault();
}

void CTaskCollection::setRequirement(RequirementPtr_t _requirement)
{
    m_requirement = _requirement;
}

RequirementPtr_t CTaskCollection::getRequirement() const
{
    return m_requirement;
}

void CTaskCollection::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& collectionPT = CTopoElement::findElement(ETopoType::COLLECTION, _name, _pt.get_child("topology"));

        setId(collectionPT.get<string>("<xmlattr>.id"));

        string requirementId = collectionPT.get<string>(TopoTypeToUseTag(ETopoType::REQUIREMENT), "");
        if (!requirementId.empty())
        {
            RequirementPtr_t newRequirement = make_shared<CRequirement>();
            newRequirement->setParent(this);
            newRequirement->initFromPropertyTree(requirementId, _pt);
            setRequirement(newRequirement);
        }

        boost::optional<const ptree&> tasksPT = collectionPT.get_child_optional("tasks");
        if (tasksPT)
        {
            for (const auto& task : tasksPT.get())
            {
                TopoElementPtr_t newElement = make_shared<CTask>();
                newElement->setParent(this);
                newElement->initFromPropertyTree(task.second.data(), _pt);
                addElement(newElement);
            }
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw runtime_error("Unable to initialize task collection " + _name + " error: " + error.what());
    }
}
