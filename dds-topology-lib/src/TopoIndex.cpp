// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoIndex.h"
// STD
#include <sstream>

using namespace std;
using namespace dds;
using namespace topology_api;

CTopoIndex::CTopoIndex()
    : m_path()
{
}

CTopoIndex::CTopoIndex(const string& _path)
    : m_path(_path)
{
}

CTopoIndex::~CTopoIndex()
{
}

string CTopoIndex::getPath() const
{
    return m_path;
}

string CTopoIndex::toString() const
{
    stringstream ss;
    ss << "TopoIndex: path=" << m_path << endl;
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoIndex& _index)
{
    _strm << _index.toString();
    return _strm;
}

bool CompareTopoIndexLess::operator()(const CTopoIndex& index1, const CTopoIndex& index2) const
{
    return index1.getPath() < index2.getPath();
}
