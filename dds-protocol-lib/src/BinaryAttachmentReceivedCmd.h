// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__BinaryAttachmentReceivedCmd__
#define __DDS__BinaryAttachmentReceivedCmd__

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
    struct SBinaryAttachmentReceivedCmd : public SBasicCmd<SBinaryAttachmentReceivedCmd>
    {
        SBinaryAttachmentReceivedCmd()
            : m_receivedFilePath()
            , m_requestedFileName()
            , m_srcCommand(0)
            , m_receivedFileSize(0)
            , m_downloadTime(0)
        {
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            size_t size(m_receivedFilePath.size() + 1);
            size += (m_requestedFileName.size() + 1);
            size += sizeof(m_srcCommand);
            size += sizeof(m_receivedFileSize);
            size += sizeof(m_downloadTime);
            return size;
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SBinaryAttachmentReceivedCmd& _val) const
        {
            return (m_receivedFilePath == _val.m_receivedFilePath && m_requestedFileName == _val.m_requestedFileName &&
                    m_srcCommand == _val.m_srcCommand && m_receivedFileSize == _val.m_receivedFileSize &&
                    m_downloadTime == _val.m_downloadTime);
        }

        std::string m_receivedFilePath;      ///> Path to the received file
        std::string m_requestedFileName;     ///> Requested name of the file
        mutable uint16_t m_srcCommand;       ///> Source command which initiated file transport
        mutable uint32_t m_receivedFileSize; ///> Number of recieved bytes
        mutable uint32_t m_downloadTime;     ///> Time spent to download file [microseconds]
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SBinaryAttachmentReceivedCmd& _val)
    {
        _stream << "receivedFilePath=" << _val.m_receivedFilePath << " requestedFileName=" << _val.m_requestedFileName
                << " receivedFileSize=" << _val.m_receivedFileSize << " downloadTime=" << _val.m_downloadTime;
        return _stream;
    }
};

#endif /* defined(__DDS__BinaryAttachmentReceivedCmd__) */
