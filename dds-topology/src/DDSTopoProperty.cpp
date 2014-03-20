// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopoProperty.h"
// STD
#include <iostream>

using namespace std;
using namespace boost::property_tree;

DDSTopoProperty::DDSTopoProperty() : DDSTopoBase()
{
    setType(DDSTopoType::TOPO_PROPERTY);
}

DDSTopoProperty::~DDSTopoProperty()
{
}
