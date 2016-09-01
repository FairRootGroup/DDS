// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "HexView.h"
#include "INet.h"
#include "ProtocolCommands.h"
#include "ProtocolMessage.h"

using namespace dds;
using namespace dds::protocol_api;
using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::INet;

CProtocolMessage::CProtocolMessage()
    : m_data(header_length)
{
}

CProtocolMessage::CProtocolMessage(uint16_t _cmd, const MiscCommon::BYTEVector_t& _data)
    : m_data(header_length)
{
    encode(_cmd, _data);
}

void CProtocolMessage::clear()
{
    m_header.clear();
    m_data.clear();
    m_data.resize(header_length);
}

void CProtocolMessage::resize(size_t _size)
{
    m_data.resize(_size);
}

const CProtocolMessage::data_t* CProtocolMessage::data() const
{
    return &m_data[0];
}

CProtocolMessage::data_t* CProtocolMessage::data()
{
    return &m_data[0];
}

size_t CProtocolMessage::length() const
{
    return header_length + m_header.m_len;
}

const CProtocolMessage::data_t* CProtocolMessage::body() const
{
    return &m_data[header_length];
}

CProtocolMessage::data_t* CProtocolMessage::body()
{
    return &m_data[header_length];
}

size_t CProtocolMessage::body_length() const
{
    return m_header.m_len;
}

////////////////////////////////////////////////////////////////////////
// return:
// 1. an exception - if the message bad/corrupted
// 2. an invalid SMessageHeader - if the message is incomplete
// 3. a valid SMessageHeader - if the message is OK
////////////////////////////////////////////////////////////////////////
bool CProtocolMessage::decode_header()
{
    SMessageHeader header;
    if (m_data.size() < header_length)
        return false;

    memcpy(&header, &m_data[0], header_length);
    header.m_crc = normalizeRead(header.m_crc);
    header.m_cmd = normalizeRead(header.m_cmd);
    header.m_len = normalizeRead(header.m_len);

    if (!header.isValid())
    {
        stringstream ss;
        ss << "the protocol message is bad or corrupted. Invalid header:\n" << BYTEVectorHexView_t(m_data);
        throw runtime_error(ss.str());
    }

    m_header = header;

    m_data.resize(length());

    // TODO: Check if the message is bigger than max_body_length

    // Empty message?
    if (0 == header.m_len)
        return true;

    return true;
}

void CProtocolMessage::_encode_message(uint16_t _cmd, const CProtocolMessage::dataContainer_t& _data)
{
    // local copy
    m_header.m_cmd = _cmd;
    m_header.m_len = _data.size();
    m_header.m_crc = m_header.getChecksum();

    // prepare data for transport
    SMessageHeader header;
    header.m_crc = normalizeWrite(m_header.m_crc);
    header.m_cmd = normalizeWrite(m_header.m_cmd);
    header.m_len = normalizeWrite(m_header.m_len);

    BYTEVector_t ret_val(header_length);
    memcpy(&ret_val[0], reinterpret_cast<unsigned char*>(&header), header_length);
    copy(_data.begin(), _data.end(), back_inserter(ret_val));
    swap(m_data, ret_val);
}

const SMessageHeader& CProtocolMessage::header() const
{
    return m_header;
}

string CProtocolMessage::toString() const
{
    stringstream ss;
    ss << "[" << g_cmdToString[m_header.m_cmd] << "] "
       << "data size (header+body): " << length() << "\n";
    static const size_t maxDataSize = 128;
    if (m_data.size() > maxDataSize)
    {
        dataContainer_t tmp_data(m_data.begin(), m_data.begin() + maxDataSize);
        ss << BYTEVectorHexView_t(tmp_data);
    }
    else
    {
        ss << BYTEVectorHexView_t(m_data);
    }
    return ss.str();
}
