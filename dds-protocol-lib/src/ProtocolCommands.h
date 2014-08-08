// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef PROTOCOLCOMMANDS_H_
#define PROTOCOLCOMMANDS_H_
// STD
#include <iterator>
#include <type_traits>
// MiscCommon
#include "def.h"
// BOOST
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#define NAME_TO_STRING(NAME) #NAME

#define REG_CMD_WITH_ATTACHMENT(cmd, attachment_class)                                                     \
    template <typename A>                                                                                  \
    struct validate_command_attachment<A, cmd>                                                             \
    {                                                                                                      \
        void operator()()                                                                                  \
        {                                                                                                  \
            static_assert(std::is_same<attachment_class, A>::value,                                        \
                          "Bad attachment to the protocol command: " #cmd " requires " #attachment_class); \
        }                                                                                                  \
    };

// define current protocol version version
const uint16_t g_protocolCommandsVersion = 1;

namespace dds
{
    enum ECmdType
    {
        // ----------- VERSION 1 --------------------
        cmdUNKNOWN = -1,
        cmdSHUTDOWN = 1,
        cmdHANDSHAKE,       // attachment: SVersionCmd
        cmdHANDSHAKE_AGENT, // attachment: SVersionCmd
        cmdSUBMIT,          // attachment: SSubmitCmd
        cmdSIMPLE_MSG,      // attachment: SSimpleMsgCmd
        cmdREPLY_HANDSHAKE_OK,
        cmdREPLY_ERR_BAD_PROTOCOL_VERSION,
        cmdREPLY_SUBMIT_OK,  // attachment: SSimpleMsgCmd
        cmdREPLY_ERR_SUBMIT, // attachment: SSimpleMsgCmd
        cmdGET_HOST_INFO,
        cmdREPLY_HOST_INFO, // attachment: SHostInfoCmd
        cmdDISCONNECT,
        cmdGED_PID,
        cmdREPLY_PID,            // attachment: SSimpleMsgCmd. The message contians the pid of the responder.
        cmdBINARY_ATTACHMENT,    // attachment: SBinanryAttachmentCmd. The message containes binary attachment.
        cmdBINARY_DOWNLOAD_STAT, // attachment: SBinaryDownloadStatCmd.
        cmdSET_UUID              // attachment: SSetUUIDCmd.

        // ----------- VERSION 2 --------------------
    };

    static std::map<uint16_t, std::string> g_cmdToString{
        { cmdUNKNOWN, NAME_TO_STRING(cmdUNKNOWN) },
        { cmdSHUTDOWN, NAME_TO_STRING(cmdSHUTDOWN) },
        { cmdHANDSHAKE, NAME_TO_STRING(cmdHANDSHAKE) },
        { cmdHANDSHAKE_AGENT, NAME_TO_STRING(cmdHANDSHAKE_AGENT) },
        { cmdSUBMIT, NAME_TO_STRING(cmdSUBMIT) },
        { cmdREPLY_HANDSHAKE_OK, NAME_TO_STRING(cmdREPLY_HANDSHAKE_OK) },
        { cmdREPLY_ERR_BAD_PROTOCOL_VERSION, NAME_TO_STRING(cmdREPLY_ERR_BAD_PROTOCOL_VERSION) },
        { cmdREPLY_SUBMIT_OK, NAME_TO_STRING(cmdREPLY_SUBMIT_OK) },
        { cmdREPLY_ERR_SUBMIT, NAME_TO_STRING(cmdREPLY_ERR_SUBMIT) },
        { cmdGET_HOST_INFO, NAME_TO_STRING(cmdGET_HOST_INFO) },
        { cmdREPLY_HOST_INFO, NAME_TO_STRING(cmdREPLY_HOST_INFO) },
        { cmdDISCONNECT, NAME_TO_STRING(cmdDISCONNECT) },
        { cmdGED_PID, NAME_TO_STRING(cmdGED_PID) },
        { cmdREPLY_PID, NAME_TO_STRING(cmdREPLY_PID) },
        { cmdBINARY_ATTACHMENT, NAME_TO_STRING(cmdBINARY_ATTACHMENT) },
        { cmdBINARY_DOWNLOAD_STAT, NAME_TO_STRING(cmdBINARY_DOWNLOAD_STAT) },
        { cmdSET_UUID, NAME_TO_STRING(cmdSET_UUID) },
    };

    //----------------------------------------------------------------------
    struct SVersionCmd;
    struct SSubmitCmd;
    struct SSimpleMsgCmd;
    struct SHostInfoCmd;
    struct SBinaryAttachmentCmd;
    struct SBinaryDownloadStatCmd;
    struct SUUIDCmd;

    template <typename A, ECmdType>
    struct validate_command_attachment;

    REG_CMD_WITH_ATTACHMENT(cmdHANDSHAKE, SVersionCmd);
    REG_CMD_WITH_ATTACHMENT(cmdHANDSHAKE_AGENT, SVersionCmd);
    REG_CMD_WITH_ATTACHMENT(cmdSUBMIT, SSubmitCmd);
    REG_CMD_WITH_ATTACHMENT(cmdSIMPLE_MSG, SSimpleMsgCmd);
    REG_CMD_WITH_ATTACHMENT(cmdREPLY_SUBMIT_OK, SSimpleMsgCmd);
    REG_CMD_WITH_ATTACHMENT(cmdREPLY_ERR_SUBMIT, SSimpleMsgCmd);
    REG_CMD_WITH_ATTACHMENT(cmdREPLY_HOST_INFO, SHostInfoCmd);
    REG_CMD_WITH_ATTACHMENT(cmdREPLY_PID, SSimpleMsgCmd);
    REG_CMD_WITH_ATTACHMENT(cmdBINARY_ATTACHMENT, SBinaryAttachmentCmd);
    REG_CMD_WITH_ATTACHMENT(cmdBINARY_DOWNLOAD_STAT, SBinaryDownloadStatCmd);
    REG_CMD_WITH_ATTACHMENT(cmdSET_UUID, SUUIDCmd);

    //----------------------------------------------------------------------

    template <class _Owner>
    struct SBasicCmd
    {
        void convertFromData(const MiscCommon::BYTEVector_t& _data)
        {
            _Owner* p = reinterpret_cast<_Owner*>(this);
            p->_convertFromData(_data);
            p->normalizeToLocal();
        }
        void convertToData(MiscCommon::BYTEVector_t* _data)
        {
            _Owner* p = reinterpret_cast<_Owner*>(this);
            p->normalizeToRemote();
            p->_convertToData(_data);
            p->normalizeToLocal();
        }
    };

    //----------------------------------------------------------------------

    struct SVersionCmd : public SBasicCmd<SVersionCmd>
    {
        SVersionCmd()
            : m_version(g_protocolCommandsVersion)
        {
        }
        void normalizeToLocal();
        void normalizeToRemote();
        size_t size() const
        {
            return sizeof(m_version);
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SVersionCmd& val) const
        {
            return (m_version == val.m_version);
        }

        uint16_t m_version;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SVersionCmd& val)
    {
        return _stream << val.m_version;
    }
    inline bool operator!=(const SVersionCmd& lhs, const SVersionCmd& rhs)
    {
        return !(lhs == rhs);
    }

    //----------------------------------------------------------------------

    struct SSimpleMsgCmd : public SBasicCmd<SSimpleMsgCmd>
    {
        SSimpleMsgCmd()
        {
        }
        void normalizeToLocal();
        void normalizeToRemote();
        size_t size() const
        {
            return (m_sMsg.size() + 1);
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SSimpleMsgCmd& val) const
        {
            return (m_sMsg == val.m_sMsg);
        }

        std::string m_sMsg;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SSimpleMsgCmd& val)
    {
        return _stream << val.m_sMsg;
    }
    inline bool operator!=(const SSimpleMsgCmd& lhs, const SSimpleMsgCmd& rhs)
    {
        return !(lhs == rhs);
    }

    //----------------------------------------------------------------------

    struct SSubmitCmd : public SBasicCmd<SSubmitCmd>
    {
        SSubmitCmd()
        {
        }
        void normalizeToLocal();
        void normalizeToRemote();
        size_t size() const
        {
            return (m_sTopoFile.size() + 1);
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SSubmitCmd& val) const
        {
            return (m_sTopoFile == val.m_sTopoFile);
        }

        std::string m_sTopoFile;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SSubmitCmd& val)
    {
        return _stream << val.m_sTopoFile;
    }
    inline bool operator!=(const SSubmitCmd& lhs, const SSubmitCmd& rhs)
    {
        return !(lhs == rhs);
    }

    //----------------------------------------------------------------------

    struct SHostInfoCmd : public SBasicCmd<SHostInfoCmd>
    {
        SHostInfoCmd()
            : m_agentPort(0)
            , m_agentPid(0)
            , m_timeStamp(0)
        {
        }
        size_t size() const
        {
            size_t size(m_username.size() + 1);
            size += m_host.size() + 1;
            size += sizeof(m_agentPort);
            size += sizeof(m_agentPid);
            size += m_version.size() + 1;
            size += m_DDSPath.size() + 1;
            size += sizeof(m_timeStamp);
            return size;
        }
        void normalizeToLocal();
        void normalizeToRemote();
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SHostInfoCmd& val) const
        {
            return (m_username == val.m_username && m_host == val.m_host && m_version == val.m_version &&
                    m_DDSPath == val.m_DDSPath && m_agentPort == val.m_agentPort && m_agentPid == val.m_agentPid &&
                    m_timeStamp == val.m_timeStamp);
        }

        std::string m_username;
        std::string m_host;
        std::string m_version;
        std::string m_DDSPath;
        uint16_t m_agentPort;
        uint32_t m_agentPid;
        uint32_t m_timeStamp; // defines a time stamp when DDS Job was submitted
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SHostInfoCmd& val)
    {
        _stream << val.m_username << ":" << val.m_host << ": " << val.m_version << ":" << val.m_DDSPath << "; agent ["
                << val.m_agentPid << "] on port " << val.m_agentPort << "; submitted on " << val.m_timeStamp;
        return _stream;
    }
    inline bool operator!=(const SHostInfoCmd& lhs, const SHostInfoCmd& rhs)
    {
        return !(lhs == rhs);
    }

    //----------------------------------------------------------------------

    struct SIdCmd : public SBasicCmd<SIdCmd>
    {
        SIdCmd()
            : m_id(0)
        {
        }
        void normalizeToLocal();
        void normalizeToRemote();
        size_t size() const
        {
            return sizeof(m_id);
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SIdCmd& _val) const
        {
            return (m_id == _val.m_id);
        }

        uint32_t m_id;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SIdCmd& _val)
    {
        return _stream << _val.m_id;
    }

    //----------------------------------------------------------------------

    struct SWnListCmd : public SBasicCmd<SWnListCmd>
    {
        SWnListCmd()
        {
        }
        size_t size() const
        {
            size_t size(0);
            MiscCommon::StringVector_t::const_iterator iter = m_container.begin();
            MiscCommon::StringVector_t::const_iterator iter_end = m_container.end();
            for (; iter != iter_end; ++iter)
            {
                size += iter->size() + 1;
            }

            return size;
        }
        void normalizeToLocal();
        void normalizeToRemote();
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SWnListCmd& val) const
        {
            return (std::equal(m_container.begin(), m_container.end(), val.m_container.begin()));
        }

        MiscCommon::StringVector_t m_container;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SWnListCmd& val)
    {
        std::ostream_iterator<std::string> output(_stream, "\n");
        std::copy(val.m_container.begin(), val.m_container.end(), output);
        return _stream;
    }

    //----------------------------------------------------------------------

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

    //----------------------------------------------------------------------

    struct SBinaryDownloadStatCmd : public SBasicCmd<SBinaryDownloadStatCmd>
    {
        SBinaryDownloadStatCmd()
            : m_recievedFileSize(0)
            , m_recievedCrc32(0)
            , m_downloadTime(0)
        {
        }
        void normalizeToLocal();
        void normalizeToRemote();
        size_t size() const
        {
            size_t size(sizeof(m_recievedFileSize));
            size += sizeof(m_recievedCrc32);
            size += sizeof(m_downloadTime);
            return size;
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SBinaryDownloadStatCmd& _val) const
        {
            return (m_recievedFileSize == _val.m_recievedFileSize && m_recievedCrc32 == _val.m_recievedCrc32 &&
                    m_downloadTime == _val.m_downloadTime);
        }

        uint32_t m_recievedFileSize; ///> Number of recieved bytes
        uint32_t m_recievedCrc32;    ///> CRC32 checksum of the recieved file
        uint32_t m_downloadTime;     ///> Time spent to download file
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SBinaryDownloadStatCmd& _val)
    {
        _stream << _val.m_recievedFileSize << " " << _val.m_recievedCrc32 << " " << _val.m_downloadTime;
        return _stream;
    }

    //----------------------------------------------------------------------

    struct SUUIDCmd : public SBasicCmd<SUUIDCmd>
    {
        SUUIDCmd()
            : m_id()
        {
        }
        void normalizeToLocal();
        void normalizeToRemote();
        size_t size() const
        {
            size_t size(boost::uuids::uuid::static_size());
            return size;
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SUUIDCmd& _val) const
        {
            return (m_id == _val.m_id);
        }

        boost::uuids::uuid m_id;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SUUIDCmd& _val)
    {
        _stream << _val.m_id;
        return _stream;
    }
}

#endif /* PROTOCOLMESSAGES_H_ */
