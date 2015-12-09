// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__BinaryAttachmentCmd__
#define __DDS__BinaryAttachmentCmd__

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
        struct SBinaryAttachmentCmd : public SBasicCmd<SBinaryAttachmentCmd>
        {
            SBinaryAttachmentCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SBinaryAttachmentCmd& _val) const;

            boost::uuids::uuid m_fileId;     ///> Unique ID of the file
            uint32_t m_offset;               ///> Offset for this piece of binary data
            uint32_t m_size;                 ///> Size of this piece of binary data
            uint32_t m_crc32;                ///> CRC checksum of this piece of binary data
            MiscCommon::BYTEVector_t m_data; ///> Piece of binary data
        };
        std::ostream& operator<<(std::ostream& _stream, const SBinaryAttachmentCmd& _val);
        bool operator!=(const SBinaryAttachmentCmd& lhs, const SBinaryAttachmentCmd& rhs);
    }
};

#endif /* defined(__DDS__BasicCmd__) */
