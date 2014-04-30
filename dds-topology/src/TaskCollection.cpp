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

void CTaskCollection::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& collectionPT = CTopoElement::findElement(ETopoType::COLLECTION, _name, _pt.get_child("topology"));

        setName(collectionPT.get<string>("<xmlattr>.name"));

        for (const auto& element : collectionPT)
        {
            if (element.first == "<xmlattr>")
                continue;
            TopoElementPtr_t newElement = CreateTopoElement(TagToTopoType(element.first));
            newElement->setParent(this);
            newElement->initFromPropertyTree(element.second.get<string>("<xmlattr>.name"), _pt);
            addElement(newElement);
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw runtime_error("Unable to initialize task collection " + _name + " error: " + error.what());
    }
}
