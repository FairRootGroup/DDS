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
    struct SBinaryAttachmentStartCmd : public SBasicCmd<SBinaryAttachmentStartCmd>
    {
        SBinaryAttachmentStartCmd()
            : m_fileId()
            , m_fileName()
            , m_fileSize(0)
            , m_fileCrc32(0)
            , m_srcCommand(0)
        {
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            size_t size(boost::uuids::uuid::static_size());
            size += (m_fileName.size() + 1);
            size += sizeof(m_fileSize);
            size += sizeof(m_fileCrc32);
            size += sizeof(m_srcCommand);
            return size;
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SBinaryAttachmentStartCmd& _val) const
        {
            return (m_fileId == _val.m_fileId && m_fileCrc32 == _val.m_fileCrc32 && m_fileName == _val.m_fileName &&
                    m_fileSize == _val.m_fileSize && m_srcCommand == _val.m_srcCommand);
        }

        boost::uuids::uuid m_fileId;   ///> Unique ID of the file
        std::string m_fileName;        ///> Name of the file
        mutable uint32_t m_fileSize;   ///> File size in bytes
        mutable uint32_t m_fileCrc32;  ///> File checksum
        mutable uint16_t m_srcCommand; ///> Source command which initiated file transport
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SBinaryAttachmentStartCmd& _val)
    {
        _stream << "fileId=" << _val.m_fileId << " fileName=" << _val.m_fileName << " fileSize=" << _val.m_fileSize
                << " fileCrc32=" << _val.m_fileCrc32;
        return _stream;
    }
};

#endif /* defined(__DDS__BinaryAttachmentStartCmd__) */
