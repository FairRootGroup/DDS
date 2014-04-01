// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopoElement.h"
#include "DDSTaskGroup.h"

using namespace std;

DDSTopoElement::DDSTopoElement()
    : DDSTopoBase()
    , m_path()
{
    setType(DDSTopoType::TOPO_ELEMENT);
}

DDSTopoElement::~DDSTopoElement()
{
}

size_t DDSTopoElement::getTotalCounterDefault() const
{
    if (getParent() != nullptr && getParent()->getType() == DDSTopoType::GROUP)
    {
        return static_cast<DDSTaskGroup*>(getParent())->getN();
    }
    return 1;
}
