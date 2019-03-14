// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoElement.h"
#include "TopoGroup.h"

using namespace std;
using namespace dds;
using namespace topology_api;

CTopoElement::CTopoElement()
    : CTopoBase()
    , m_path()
{
    setType(CTopoBase::EType::TOPO_ELEMENT);
}

CTopoElement::~CTopoElement()
{
}

size_t CTopoElement::getTotalCounterDefault() const
{
    if (getParent() != nullptr && getParent()->getType() == CTopoBase::EType::GROUP)
    {
        return static_cast<CTopoGroup*>(getParent())->getN();
    }
    return 1;
}
