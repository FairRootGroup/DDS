// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "ProtocolMessage.h"
#include "HexView.h"
#include "INet.h"

using namespace dds;
using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::INet;

CProtocolMessage::CProtocolMessage()
    : m_data(header_length + max_body_length)
{
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

void CProtocolMessage::body_length(size_t new_length)
{
    /*    body_length_ = new_length;
        if (body_length_ > max_body_length)
            body_length_ = max_body_length;*/
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
    if (!header.isValid())
    {
        stringstream ss;
        ss << "the protocol message is bad or corrupted. Invalid header:\n" << BYTEVectorHexView_t(m_data);
        throw runtime_error(ss.str());
    }

    header.m_cmd = _normalizeRead16(header.m_cmd);
    header.m_len = _normalizeRead32(header.m_len);

    m_header = header;

    // TODO: Check if the message is bigger than max_body_length

    // Empty message?
    if (0 == header.m_len)
        return true;

    //  BYTEVector_t::const_iterator iter = _msg.begin() + HEADER_SIZE;
    //  copy(iter, iter + header.m_len, back_inserter(*_data));

    return true;
}

void CProtocolMessage::encode_message(uint16_t _cmd, const CProtocolMessage::dataContainer_t& _data)
{
    SMessageHeader header;
    strncpy(header.m_sign, g_CmdSign, sizeof(header.m_sign));
    header.m_cmd = _normalizeWrite16(_cmd);
    header.m_len = _normalizeWrite32(_data.size());

    dataContainer_t ret_val(header_length);
    memcpy(&ret_val[0], reinterpret_cast<unsigned char*>(&header), header_length);
    copy(_data.begin(), _data.end(), back_inserter(ret_val));
    swap(m_data, ret_val);
}
