// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTaskCollection.h"
#include "DDSTask.h"
#include "DDSTopoFactory.h"
#include "DDSTopoUtils.h"

using namespace std;
using namespace boost::property_tree;

DDSTaskCollection::DDSTaskCollection() : DDSTaskContainer()
{
    setType(DDSTopoType::COLLECTION);
}

DDSTaskCollection::~DDSTaskCollection()
{
}

size_t DDSTaskCollection::getNofTasks() const
{
    return getNofTasksDefault();
}

size_t DDSTaskCollection::getTotalNofTasks() const
{
    return getNofTasksDefault();
}

size_t DDSTaskCollection::getMinRequiredNofTasks() const
{
    return getNofTasksDefault();
}

size_t DDSTaskCollection::getTotalCounter() const
{
    return getTotalCounterDefault();
}

void DDSTaskCollection::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& collectionPT = DDSTopoElement::findElement(DDSTopoType::COLLECTION, _name, _pt.get_child("topology"));

        setName(collectionPT.get<string>("<xmlattr>.name"));

        for (const auto& element : collectionPT)
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
        throw runtime_error("Unable to initialize task collection " + _name + " error: " + error.what());
    }
}
