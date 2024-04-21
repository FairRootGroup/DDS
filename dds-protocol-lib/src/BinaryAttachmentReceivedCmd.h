// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__BinaryAttachmentReceivedCmd__
#define __DDS__BinaryAttachmentReceivedCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SBinaryAttachmentReceivedCmd : public SBasicCmd<SBinaryAttachmentReceivedCmd>
        {
            SBinaryAttachmentReceivedCmd();
            size_t size() const;
            void _convertFromData(const dds::misc::BYTEVector_t& _data);
            void _convertToData(dds::misc::BYTEVector_t* _data) const;
            bool operator==(const SBinaryAttachmentReceivedCmd& _val) const;

            std::string m_receivedFilePath;  ///< Path to the received file
            std::string m_requestedFileName; ///< Requested name of the file
            uint16_t m_srcCommand;           ///< Source command which initiated file transport
            uint32_t m_receivedFileSize;     ///< Number of received bytes
            uint32_t m_downloadTime;         ///< Time spent to download file [microseconds]
        };
        std::ostream& operator<<(std::ostream& _stream, const SBinaryAttachmentReceivedCmd& _val);
        bool operator!=(const SBinaryAttachmentReceivedCmd& lhs, const SBinaryAttachmentReceivedCmd& rhs);
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__BinaryAttachmentReceivedCmd__) */
