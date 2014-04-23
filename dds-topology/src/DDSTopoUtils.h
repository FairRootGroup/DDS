// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopoUtils__
#define __DDS__DDSTopoUtils__

// DDS
#include "DDSTopoElement.h"
#include "DDSPort.h"
// STD
#include <string>

std::string DDSTopoTypeToTag(DDSTopoType _type);

DDSTopoType DDSTagToTopoType(const std::string& _name);

DDSPortType DDSStringToPortType(const std::string& _name);

#endif /* defined(__DDS__DDSTopoUtils__) */
