// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__BinaryAttachmentStartCmd__
#define __DDS__BinaryAttachmentStartCmd__

// DDS
#include "BasicCmd.h"
// BOOST
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#pragma clang diagnostic pop

namespace dds
{
    namespace protocol_api
    {
        struct SBinaryAttachmentStartCmd : public SBasicCmd<SBinaryAttachmentStartCmd>
        {
            SBinaryAttachmentStartCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SBinaryAttachmentStartCmd& _val) const;

            boost::uuids::uuid m_fileId; ///< Unique ID of the file
            std::string m_fileName;      ///< Name of the file
            uint32_t m_fileSize;         ///< File size in bytes
            uint32_t m_fileCrc32;        ///< File checksum
            uint16_t m_srcCommand;       ///< Source command which initiated file transport
        };
        std::ostream& operator<<(std::ostream& _stream, const SBinaryAttachmentStartCmd& _val);
        bool operator!=(const SBinaryAttachmentStartCmd& lhs, const SBinaryAttachmentStartCmd& rhs);
    }
}

#endif /* defined(__DDS__BinaryAttachmentStartCmd__) */
