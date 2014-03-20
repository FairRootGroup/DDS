// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopoFactory__
#define __DDS__DDSTopoFactory__

#include "DDSTopoBase.h"
#include "DDSTopoElement.h"
#include "DDSTopoProperty.h"

DDSTopoElementPtr_t DDSCreateTopoElement(DDSTopoType _type);

DDSTopoPropertyPtr_t DDSCreateTopoProperty(DDSTopoType _type);

DDSTopoBasePtr_t DDSCreateTopoBase(DDSTopoType _type);

#endif /* defined(__DDS__DDSTopoFactory__) */
