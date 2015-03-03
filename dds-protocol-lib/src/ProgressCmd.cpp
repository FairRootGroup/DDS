//
//  ProgressCmd.cpp
//  DDS
//
//  Created by Andrey Lebedev on 27/01/15.
//
//

#include "ProgressCmd.h"
// MiscCommon
#include "INet.h"

using namespace std;
using namespace dds;
namespace inet = MiscCommon::INet;

void SProgressCmd::normalizeToLocal() const
{
    m_completed = inet::normalizeRead(m_completed);
    m_total = inet::normalizeRead(m_total);
    m_errors = inet::normalizeRead(m_errors);
    m_time = inet::normalizeRead(m_time);
}

void SProgressCmd::normalizeToRemote() const
{
    m_completed = inet::normalizeWrite(m_completed);
    m_total = inet::normalizeWrite(m_total);
    m_errors = inet::normalizeWrite(m_errors);
    m_time = inet::normalizeWrite(m_time);
}

void SProgressCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    if (_data.size() < size())
    {
        stringstream ss;
        ss << "SProgressCmd: Protocol message data is too short, expected " << size() << " received " << _data.size();
        throw runtime_error(ss.str());
    }

    size_t idx(0);
    inet::readData(&m_completed, &_data, &idx);
    inet::readData(&m_total, &_data, &idx);
    inet::readData(&m_errors, &_data, &idx);
    inet::readData(&m_time, &_data, &idx);
}

void SProgressCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    inet::pushData(m_completed, _data);
    inet::pushData(m_total, _data);
    inet::pushData(m_errors, _data);
    inet::pushData(m_time, _data);
}
