// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoProperty.h"
// STD
#include <iostream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;

CTopoProperty::CTopoProperty()
    : CTopoBase()
{
    setType(ETopoType::TOPO_PROPERTY);
}

CTopoProperty::~CTopoProperty()
{
}
