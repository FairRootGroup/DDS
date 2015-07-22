// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoElement.h"
#include "TaskGroup.h"

using namespace std;
using namespace dds;
using namespace topology_api;

CTopoElement::CTopoElement()
    : CTopoBase()
    , m_path()
{
    setType(ETopoType::TOPO_ELEMENT);
}

CTopoElement::~CTopoElement()
{
}

size_t CTopoElement::getTotalCounterDefault() const
{
    if (getParent() != nullptr && getParent()->getType() == ETopoType::GROUP)
    {
        return static_cast<CTaskGroup*>(getParent())->getN();
    }
    return 1;
}
