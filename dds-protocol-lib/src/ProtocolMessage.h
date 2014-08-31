// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__ProtocolMessage__
#define __DDS__ProtocolMessage__
// DDS
#include "def.h"
#include "ProtocolCommands.h"
// STD
#include <cstring>

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
        typedef std::shared_ptr<CProtocolMessage> protocolMessagePtr_t;

        enum
        {
            cmd_sign_length = 2
        };
        enum
        {
            header_length = sizeof(SMessageHeader)
        };
        /*  enum
          {
              max_body_length = 512
          };*/

      public:
        CProtocolMessage();

      public:
        template <ECmdType _cmd, typename A>
        void encodeWithAttachment(A& _cmdAttachment)
        {
            // compile time check that command is being encodded with the proper attahcment class
            validate_command_attachment<A, _cmd>()();

            MiscCommon::BYTEVector_t data;
            _cmdAttachment.convertToData(&data);
            _encode_message(_cmd, data);
        }
        template <ECmdType _cmd>
        void encode()
        {
            MiscCommon::BYTEVector_t data;
            _encode_message(_cmd, data);
        }

        void clear();
        void resize(size_t _size); // FIXME: Used in tests to allocate memory for m_data.
        const data_t* data() const;
        data_t* data();
        size_t length() const;
        const data_t* body() const;
        data_t* body();
        size_t body_length() const;
        bool decode_header();
        const SMessageHeader header() const;
        std::string toString() const;
        const dataContainer_t dataToContainer() const
        {
            return m_data;
        }
        const dataContainer_t bodyToContainer() const
        {
            dataContainer_t buf(body(), body() + body_length());
            return buf;
        }

      private:
        void _encode_message(uint16_t _cmd, const dataContainer_t& _data);

      private:
        dataContainer_t m_data; /// the whole data buffer, whcih includes the header and the msg body
        SMessageHeader m_header;
    };
}

#endif /* defined(__DDS__ProtocolMessage__) */
