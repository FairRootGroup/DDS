// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopology.h"
// STD
#include <string>

using namespace std;

DDSTopology::DDSTopology()
{
}

DDSTopology::~DDSTopology()
{
}

DDSTaskGroupPtr_t DDSTopology::getMainGroup() const
{
    return m_main;
}

string DDSTopology::toString() const
{
    stringstream ss;

    return ss.str();
}

ostream& operator<<(ostream& _strm, const DDSTopology& _topology)
{
    _strm << _topology.toString();
    return _strm;
}
