// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "Index.h"
// STD
#include <sstream>

using namespace std;
using namespace dds;

CIndex::CIndex()
    : m_path()
{
}

CIndex::CIndex(const string& _path)
    : m_path(_path)
{
}

CIndex::~CIndex()
{
}

string CIndex::getPath() const
{
    return m_path;
}

string CIndex::toString() const
{
    stringstream ss;
    ss << "Index: path=" << m_path << endl;
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CIndex& _index)
{
    _strm << _index.toString();
    return _strm;
}

bool CompareIndexLess::operator()(const CIndex& index1, const CIndex& index2) const
{
    return index1.getPath() < index2.getPath();
}
