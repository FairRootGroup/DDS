// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef PROTOCOLCOMMANDS_H_
#define PROTOCOLCOMMANDS_H_
// STD
#include <iterator>
// MiscCommon
#include "def.h"
// pod-agetn
#include "Protocol.h"

// v1
const uint16_t g_protocolCommandsVersion = 1;

namespace dds
{
    enum ECmdType
    {
        // ----------- VERSION 1 --------------------
        cmdUNKNOWN = -1,
        cmdVERSION = 1,
        cmdUI_CONNECT,
        cmdUI_CONNECT_READY,
        cmdHOST_INFO,
        cmdGET_HOST_INFO,
        cmdUSE_PACKETFORWARDING_PROOF,
        cmdUSE_DIRECT_PROOF,
        cmdSHUTDOWN,
        cmdGET_ID,
        cmdID,
        cmdSET_ID,
        cmdGET_WRK_NUM,  // request a number of PROOF workers, which pod wn want to spawn
        cmdWRK_NUM,      // answer with number of PROOF wns,
        cmdGET_WNs_LIST, // request a list of available workers
        cmdWNs_LIST      // return a list of available workers

        // ----------- VERSION 2 --------------------
    };

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

    //----------------------------------------------------------------------

    struct SHostInfoCmd : public SBasicCmd<SHostInfoCmd>
    {
        SHostInfoCmd()
            : m_xpdPort(0)
            , m_xpdPid(0)
            , m_agentPort(0)
            , m_agentPid(0)
            , m_timeStamp(0)
        {
        }
        size_t size() const
        {
            size_t size(m_username.size() + 1);
            size += m_host.size() + 1;
            size += sizeof(m_xpdPort);
            size += sizeof(m_xpdPid);
            size += sizeof(m_agentPort);
            size += sizeof(m_agentPid);
            size += m_version.size() + 1;
            size += m_PoDPath.size() + 1;
            size += sizeof(m_timeStamp);
            return size;
        }
        void normalizeToLocal();
        void normalizeToRemote();
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SHostInfoCmd& val) const
        {
            return (m_username == val.m_username && m_host == val.m_host && m_version == val.m_version && m_PoDPath == val.m_PoDPath &&
                    m_xpdPort == val.m_xpdPort && m_xpdPid == val.m_xpdPid && m_agentPort == val.m_agentPort && m_agentPid == val.m_agentPid &&
                    m_timeStamp == val.m_timeStamp);
        }

        std::string m_username;
        std::string m_host;
        std::string m_version;
        std::string m_PoDPath;
        uint16_t m_xpdPort;
        uint32_t m_xpdPid;
        uint16_t m_agentPort;
        uint32_t m_agentPid;
        uint32_t m_timeStamp; // defines a time stamp when PoD Job was submitted
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SHostInfoCmd& val)
    {
        _stream << val.m_username << ":" << val.m_host << ": [" << val.m_xpdPid << "] " << val.m_xpdPort << ":" << val.m_version << ":" << val.m_PoDPath
                << "; agent [" << val.m_agentPid << "] on port " << val.m_agentPort << "; submitted on " << val.m_timeStamp;
        return _stream;
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
}

#endif /* PROTOCOLMESSAGES_H_ */
