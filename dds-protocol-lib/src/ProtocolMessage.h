// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__ProtocolMessage__
#define __DDS__ProtocolMessage__
// DDS
#include "def.h"

namespace dds
{
    // a very simple protocol
    // | <DDS> (6) char | CMD (2) uint16_t | LEN (4) uint32_t | DATA (LEN) unsigned char |
    const char* const g_CmdSign = "<DDS>";
    enum
    {
        header_sign_length = 6
    };

    //----------------------------------------------------------------------

    struct SMessageHeader
    {
        SMessageHeader()
            : m_cmd(0)
            , m_len(0)
        {
            m_sign[0] = '\0';
        }
        char m_sign[header_sign_length];
        uint16_t m_cmd;
        uint32_t m_len;

        bool isValid() const
        {
            return (strcmp(m_sign, g_CmdSign) == 0);
        }
        void clear()
        {
            m_sign[0] = '\0';
            m_cmd = 0;
            m_len = 0;
        }
    };

    //----------------------------------------------------------------------

    class CProtocolMessage
    {
      public:
        typedef MiscCommon::BYTEVector_t dataContainer_t;
        typedef dataContainer_t::value_type data_t;

        enum
        {
            cmd_sign_length = 2
        };
        enum
        {
            header_length = sizeof(SMessageHeader)
        };
        enum
        {
            max_body_length = 512
        };

      public:
        CProtocolMessage();

      public:
        const data_t* data() const;
        data_t* data();
        size_t length() const;
        const data_t* body() const;
        data_t* body();
        size_t body_length() const;
        void body_length(size_t new_length);
        bool decode_header();
        void encode_message(uint16_t _cmd, const dataContainer_t& _data);
        const SMessageHeader header() const;

      private:
        dataContainer_t m_data; /// the whole data buffer, whcih includes the header and the msg body
        SMessageHeader m_header;
    };
}

#endif /* defined(__DDS__ProtocolMessage__) */
