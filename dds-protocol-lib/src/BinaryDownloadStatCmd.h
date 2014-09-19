// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__BinaryDownloadStatCmd__
#define __DDS__BinaryDownloadStatCmd__

// DDS
#include "BasicCmd.h"

// namespace dds
//{
//    struct SBinaryDownloadStatCmd : public SBasicCmd<SBinaryDownloadStatCmd>
//    {
//        SBinaryDownloadStatCmd()
//            : m_recievedFileSize(0)
//            , m_recievedCrc32(0)
//            , m_downloadTime(0)
//            , m_srcCommand(0)
//        {
//        }
//        void normalizeToLocal() const;
//        void normalizeToRemote() const;
//        size_t size() const
//        {
//            size_t size(sizeof(m_recievedFileSize));
//            size += sizeof(m_recievedCrc32);
//            size += sizeof(m_downloadTime);
//            size += sizeof(m_srcCommand);
//            return size;
//        }
//        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
//        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
//        bool operator==(const SBinaryDownloadStatCmd& _val) const
//        {
//            return (m_recievedFileSize == _val.m_recievedFileSize && m_recievedCrc32 == _val.m_recievedCrc32 &&
//                    m_downloadTime == _val.m_downloadTime);
//        }
//
//        mutable uint32_t m_recievedFileSize; ///> Number of recieved bytes
//        mutable uint32_t m_recievedCrc32;    ///> CRC32 checksum of the recieved file
//        mutable uint32_t m_downloadTime;     ///> Time spent to download file [microseconds]
//        mutable uint16_t m_srcCommand;       ///> Source command which initiated file transport
//    };
//    inline std::ostream& operator<<(std::ostream& _stream, const SBinaryDownloadStatCmd& _val)
//    {
//        _stream << _val.m_recievedFileSize << " " << _val.m_recievedCrc32 << " " << _val.m_downloadTime;
//        return _stream;
//    }
//};

#endif /* defined(__DDS__BinaryDownloadStatCmd__) */
