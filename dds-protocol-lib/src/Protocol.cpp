// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "Protocol.h"
// API
#include <sys/socket.h>
// MiscCommon
#include "ErrorCode.h"
#include "INet.h"
#include "HexView.h"

using namespace std;
using namespace dds;
using namespace MiscCommon;
using namespace MiscCommon::INet;

//----------------------------------------------------------------------

BYTEVector_t dds::createMsg(uint16_t _cmd, const BYTEVector_t& _data)
{
    SMessageHeader header;
    strncpy(header.m_sign, g_CmdSign, sizeof(header.m_sign));
    header.m_cmd = _normalizeWrite16(_cmd);
    header.m_len = _normalizeWrite32(_data.size());

    BYTEVector_t ret_val(HEADER_SIZE);
    memcpy(&ret_val[0], reinterpret_cast<unsigned char*>(&header), HEADER_SIZE);
    copy(_data.begin(), _data.end(), back_inserter(ret_val));

    return ret_val;
}

////////////////////////////////////////////////////////////////////////
// return:
// 1. an exception - if the message bad/corrupted
// 2. an invalid SMessageHeader - if the message is incomplete
// 3. a valid SMessageHeader - if the message is OK
////////////////////////////////////////////////////////////////////////
SMessageHeader dds::parseMsg(BYTEVector_t* _data, const BYTEVector_t& _msg)
{
    SMessageHeader header;
    if (_msg.size() < HEADER_SIZE)
        return SMessageHeader();

    memcpy(&header, &_msg[0], HEADER_SIZE);
    if (!header.isValid())
    {
        stringstream ss;
        ss << "the protocol message is bad or corrupted. Invalid header:\n" << BYTEVectorHexView_t(_msg);
        throw runtime_error(ss.str());
    }

    header.m_cmd = _normalizeRead16(header.m_cmd);
    header.m_len = _normalizeRead32(header.m_len);

    if (0 == header.m_len)
        return header;

    BYTEVector_t::const_iterator iter = _msg.begin() + HEADER_SIZE;
    copy(iter, iter + header.m_len, back_inserter(*_data));

    return header;
}

//----------------------------------------------------------------------

CProtocol::CProtocol()
{
}

CProtocol::~CProtocol()
{
}
////////////////////////////////////////////////////////////////////////
// memberof to silence doxygen warning:
// warning: no matching class member found for
// This happens because doxygen is not handling namespaces in arguments properly
//
// @memberof dds::CProtocol
//
//
////////////////////////////////////////////////////////////////////////
SMessageHeader CProtocol::getMsg(BYTEVector_t* _data) const
{
    copy(m_curDATA.begin(), m_curDATA.end(), back_inserter(*_data));
    return m_msgHeader;
}

CProtocol::EStatus_t CProtocol::read(int _socket)
{
    BYTEVector_t tmp_buf(MAX_MSG_SIZE);
    while (true)
    {
        // need to read more to complete the header
        // we use read (instead of recv) to allow non socket transports
        const ssize_t bytes_read = ::read(_socket, &tmp_buf[0], MAX_MSG_SIZE);
        if (0 == bytes_read)
            return stDISCONNECT;

        if (bytes_read < 0)
        {
            if (ECONNRESET == errno || ENOTCONN == errno)
                return stDISCONNECT;

            if (EAGAIN == errno || EWOULDBLOCK == errno)
                return stAGAIN;

            throw MiscCommon::system_error("Error occurred while reading protocol message.");
        }

        copy(tmp_buf.begin(), tmp_buf.begin() + bytes_read, back_inserter(m_buffer));

        if (bytes_read < MAX_MSG_SIZE)
            break;
    }

    return stOK;
}

bool CProtocol::checkoutNextMsg()
{
    try
    {
        m_curDATA.clear();
        m_msgHeader = parseMsg(&m_curDATA, m_buffer);
        if (!m_msgHeader.isValid())
            return false;

        // delete the message from the buffer
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + HEADER_SIZE + m_msgHeader.m_len);
    }
    catch (...)
    {
        // TODO: Clear only until there is another <DDS_CMD> found
        m_buffer.clear();
        throw;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////
// memberof to silence doxygen warning:
// warning: no matching class member found for
// This happens because doxygen is not handling namespaces in arguments properly
//
// @memberof dds::CProtocol
//
//
////////////////////////////////////////////////////////////////////////
void CProtocol::write(int _socket, uint16_t _cmd, const BYTEVector_t& _data) const
{
    BYTEVector_t msg(createMsg(_cmd, _data));
    sendall(_socket, &msg[0], msg.size(), 0);
}

////////////////////////////////////////////////////////////////////////
// memberof to silence doxygen warning:
// warning: no matching class member found for
// This happens because doxygen is not handling namespaces in arguments properly
//
// @memberof dds::CProtocol
//
//
////////////////////////////////////////////////////////////////////////
void CProtocol::writeSimpleCmd(int _socket, uint16_t _cmd) const
{
    BYTEVector_t data;
    write(_socket, _cmd, data);
}
