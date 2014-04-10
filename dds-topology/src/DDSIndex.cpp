// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSIndex.h"
// STD
#include <sstream>

using namespace std;

DDSIndex::DDSIndex()
    : m_path()
{
}

DDSIndex::DDSIndex(const string& _path)
    : m_path(_path)
{
}

DDSIndex::~DDSIndex()
{
}

string DDSIndex::getPath() const
{
    return m_path;
}

string DDSIndex::toString() const
{
    stringstream ss;
    ss << "DDSIndex: path=" << m_path << endl;
    return ss.str();
}

ostream& operator<<(ostream& _strm, const DDSIndex& _index)
{
    _strm << _index.toString();
    return _strm;
}

bool DDSCompareIndexLess::operator()(const DDSIndex& index1, const DDSIndex& index2) const
{
    return index1.getPath() < index2.getPath();
}
