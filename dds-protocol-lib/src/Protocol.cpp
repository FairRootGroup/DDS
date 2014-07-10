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

/*CProtocol::CProtocol()
{
}

CProtocol::~CProtocol()
{
}*/
////////////////////////////////////////////////////////////////////////
// memberof to silence doxygen warning:
// warning: no matching class member found for
// This happens because doxygen is not handling namespaces in arguments properly
//
// @memberof dds::CProtocol
//
//
////////////////////////////////////////////////////////////////////////
/*SMessageHeader CProtocol::getMsg(BYTEVector_t* _data) const
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
}*/

////////////////////////////////////////////////////////////////////////
// memberof to silence doxygen warning:
// warning: no matching class member found for
// This happens because doxygen is not handling namespaces in arguments properly
//
// @memberof dds::CProtocol
//
//
////////////////////////////////////////////////////////////////////////
/*void CProtocol::write(int _socket, uint16_t _cmd, const BYTEVector_t& _data) const
{
    BYTEVector_t msg(createMsg(_cmd, _data));
    sendall(_socket, &msg[0], msg.size(), 0);
}*/

////////////////////////////////////////////////////////////////////////
// memberof to silence doxygen warning:
// warning: no matching class member found for
// This happens because doxygen is not handling namespaces in arguments properly
//
// @memberof dds::CProtocol
//
//
////////////////////////////////////////////////////////////////////////
/*void CProtocol::writeSimpleCmd(int _socket, uint16_t _cmd) const
{
    BYTEVector_t data;
    write(_socket, _cmd, data);
}*/
