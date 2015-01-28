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
    m_completed = inet::_normalizeRead32(m_completed);
    m_total = inet::_normalizeRead32(m_total);
    m_errors = inet::_normalizeRead32(m_errors);
}

void SProgressCmd::normalizeToRemote() const
{
    m_completed = inet::_normalizeWrite32(m_completed);
    m_total = inet::_normalizeWrite32(m_total);
    m_errors = inet::_normalizeWrite32(m_errors);
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
    m_completed = _data[idx++];
    m_completed += (_data[idx++] << 8);
    m_completed += (_data[idx++] << 16);
    m_completed += (_data[idx] << 24);

    ++idx;
    m_total = _data[idx++];
    m_total += (_data[idx++] << 8);
    m_total += (_data[idx++] << 16);
    m_total += (_data[idx] << 24);

    ++idx;
    m_errors = _data[idx++];
    m_errors += (_data[idx++] << 8);
    m_errors += (_data[idx++] << 16);
    m_errors += (_data[idx] << 24);
}

void SProgressCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    _data->push_back(m_completed & 0xFF);
    _data->push_back(m_completed >> 8);
    _data->push_back(m_completed >> 16);
    _data->push_back(m_completed >> 24);

    _data->push_back(m_total & 0xFF);
    _data->push_back(m_total >> 8);
    _data->push_back(m_total >> 16);
    _data->push_back(m_total >> 24);

    _data->push_back(m_errors & 0xFF);
    _data->push_back(m_errors >> 8);
    _data->push_back(m_errors >> 16);
    _data->push_back(m_errors >> 24);
}
