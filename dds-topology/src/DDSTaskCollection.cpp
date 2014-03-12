// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "DDSTaskCollection.h"

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
    initFromPropertyTreeDefault(_name, _pt);
}
