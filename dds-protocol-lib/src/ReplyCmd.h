// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__ReplyCmd__
#define __DDS__ReplyCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SReplyCmd : public SBasicCmd<SReplyCmd>
        {
            enum class EStatusCode : uint16_t
            {
                OK = 1,
                ERROR
            };

            SReplyCmd();
            SReplyCmd(const std::string& _msg, uint16_t _statusCode, uint16_t _returnCode, uint16_t _srcCommand);
            size_t size() const;
            void _convertFromData(const dds::misc::BYTEVector_t& _data);
            void _convertToData(dds::misc::BYTEVector_t* _data) const;
            bool operator==(const SReplyCmd& val) const;

            uint16_t m_statusCode;
            uint16_t m_returnCode;
            uint16_t m_srcCommand;
            std::string m_sMsg;
        };
        std::ostream& operator<<(std::ostream& _stream, const SReplyCmd& val);
        bool operator!=(const SReplyCmd& lhs, const SReplyCmd& rhs);
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__ReplyCmd__) */
