// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__BinaryAttachmentCmd__
#define __DDS__BinaryAttachmentCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    struct SBinaryAttachmentCmd : public SBasicCmd<SBinaryAttachmentCmd>
    {
        SBinaryAttachmentCmd()
            : m_fileName()
            , m_fileSize(0)
            , m_crc32(0)
            , m_fileData()
        {
        }
        void normalizeToLocal();
        void normalizeToRemote();
        size_t size() const
        {
            size_t size(m_fileName.size() + 1);
            size += sizeof(m_fileSize);
            size += sizeof(m_crc32);
            size += m_fileData.size();
            return size;
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SBinaryAttachmentCmd& _val) const
        {
            unsigned int i = 0;
            for (auto c : _val.m_fileData)
            {
                if (m_fileData[i++] != c)
                    return false;
            }
            return (m_crc32 == _val.m_crc32 && m_fileName == _val.m_fileName && m_fileSize == _val.m_fileSize);
        }

        std::string m_fileName;              ///> Name of the file
        uint32_t m_fileSize;                 ///> File size in bytes
        uint32_t m_crc32;                    ///> File checksum
        MiscCommon::BYTEVector_t m_fileData; ///> Binary data
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SBinaryAttachmentCmd& _val)
    {
        _stream << _val.m_fileName << " " << _val.m_fileSize << " " << _val.m_crc32 << " ";
        for (const auto& c : _val.m_fileData)
        {
            _stream << c;
        }
        return _stream;
    }
};

#endif /* defined(__DDS__BasicCmd__) */
