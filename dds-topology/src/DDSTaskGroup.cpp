// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "DDSTaskGroup.h"

using namespace std;
using namespace boost::property_tree;

DDSTaskGroup::DDSTaskGroup() : DDSTaskContainer()
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
    initFromPropertyTreeDefault(_name, _pt);
}