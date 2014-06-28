// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef PROTOCOL_H_
#define PROTOCOL_H_
//=============================================================================
// STD
#include <cstring>
// API
#include <arpa/inet.h>
// MiscCommon
#include "def.h"
//=============================================================================
namespace dds
{
    //=============================================================================
    // a very simple protocol
    // | <POD_CMD> (10) char | CMD (2) uint16_t | LEN (4) uint32_t | DATA (LEN) unsigned char |
    struct SMessageHeader
    {
        SMessageHeader()
            : m_cmd(0)
            , m_len(0)
        {
            m_sign[0] = '\0';
        }
        char m_sign[10];
        uint16_t m_cmd;
        uint32_t m_len;

        bool isValid() const
        {
            return (strcmp(m_sign, "<POD_CMD>") == 0);
        }
        void clear()
        {
            m_sign[0] = '\0';
            m_cmd = 0;
            m_len = 0;
        }
    };
    //=============================================================================
    MiscCommon::BYTEVector_t createMsg(uint16_t _cmd, const MiscCommon::BYTEVector_t& _data);
    //=============================================================================
    SMessageHeader parseMsg(MiscCommon::BYTEVector_t* _data, const MiscCommon::BYTEVector_t& _msg);
    //=============================================================================
    /**
     *
     * @brief The protocol low level class
     *
     */
    class CProtocol
    {
      public:
        CProtocol();
        virtual ~CProtocol();

        typedef enum EStatus
        { stOK = 0,
          stDISCONNECT = 1,
          stAGAIN = 2 } EStatus_t;

        EStatus_t read(int _socket);
        void write(int _socket, uint16_t _cmd, const MiscCommon::BYTEVector_t& _data) const;
        void writeSimpleCmd(int _socket, uint16_t _cmd) const;
        SMessageHeader getMsg(MiscCommon::BYTEVector_t* _data) const;
        bool checkoutNextMsg();

      private:
        MiscCommon::BYTEVector_t m_buffer;

        SMessageHeader m_msgHeader;
        MiscCommon::BYTEVector_t m_curDATA;
    };
}

#endif /* PROTOCOL_H_ */
