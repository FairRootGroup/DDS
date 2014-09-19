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
    struct SBinaryAttachmentCmd : public SBasicCmd<SBinaryAttachmentCmd>
    {
        SBinaryAttachmentCmd()
            : m_fileId()
            , m_offset(0)
            , m_size(0)
            , m_crc32(0)
            , m_data()
        {
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            size_t size(boost::uuids::uuid::static_size());
            size += m_data.size();
            size += sizeof(m_offset);
            size += sizeof(m_size);
            size += sizeof(m_crc32);
            return size;
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SBinaryAttachmentCmd& _val) const
        {
            unsigned int i = 0;
            for (auto c : _val.m_data)
            {
                if (m_data[i++] != c)
                    return false;
            }
            return (m_fileId == _val.m_fileId && m_offset == _val.m_offset && m_size == _val.m_size &&
                    m_crc32 == _val.m_crc32);
        }

        boost::uuids::uuid m_fileId;     ///> Unique ID of the file
        mutable uint32_t m_offset;       ///> Offset for this piece of binary data
        mutable uint32_t m_size;         ///> Size of this piece of binary data
        mutable uint32_t m_crc32;        ///> CRC checksum of this piece of binary data
        MiscCommon::BYTEVector_t m_data; ///> Piece of binary data
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SBinaryAttachmentCmd& _val)
    {
        _stream << "fileId=" << _val.m_fileId << " offset=" << _val.m_offset << " size=" << _val.m_size
                << " crc32=" << _val.m_crc32;
        for (const auto& c : _val.m_data)
        {
            _stream << c;
        }
        return _stream;
    }
};

#endif /* defined(__DDS__BasicCmd__) */
