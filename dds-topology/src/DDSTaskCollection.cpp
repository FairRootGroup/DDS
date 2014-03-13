// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "DDSTaskCollection.h"
#include "DDSTask.h"

using namespace std;
using namespace boost::property_tree;

DDSTaskCollection::DDSTaskCollection() : DDSTaskContainer()
{
    setType(DDSTopoElementType::COLLECTION);
}

DDSTaskCollection::~DDSTaskCollection()
{
}

size_t DDSTaskCollection::getNofTasks() const
{
    return getNofTasksDefault();
}

void DDSTaskCollection::initFromPropertyTree(const string& _name, const ptree& _pt)
{
    try
    {
        const ptree& collectionPT = DDSTopoElement::findElement("collection", _name, _pt);

        setName(collectionPT.get<string>("<xmlattr>.name"));
        for (const auto& task : collectionPT)
        {
            if (task.first == "task")
            {
                DDSTaskPtr_t newTask = make_shared<DDSTask>();
                newTask->initFromPropertyTree(task.second.get<string>("<xmlattr>.name"), _pt);
                addElement(newTask);
            }
        }
    }
    catch (ptree_error& error)
    {
        cout << "ptree_error: " << error.what() << endl;
        throw logic_error("Unable to initialize task " + _name);
    }
    catch (logic_error& error)
    {
        throw logic_error("Unable to initialize task " + _name);
    }
}
