// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "UUIDCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

void SUUIDCmd::normalizeToLocal()
{
}

void SUUIDCmd::normalizeToRemote()
{
}

void SUUIDCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    copy(_data.begin(), _data.end(), m_id.begin());
}

void SUUIDCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    copy(m_id.begin(), m_id.end(), back_inserter(*_data));
}
