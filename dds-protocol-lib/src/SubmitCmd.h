// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__SubmitCmd__
#define __DDS__SubmitCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SSubmitCmd : public SBasicCmd<SSubmitCmd>
        {
            // a list of supported RMS
            // We define this enum here to make a strong binding between protocol and RMS type code.
            // Just in case if code of any of the supported RMS is changed (enum was re-ordered), then protocol version
            // should also be changed.
            enum ERmsType
            {
                UNKNOWN = -1,
                SSH = 0,
                LOCALHOST = 1
            };
            std::map<uint16_t, std::string> RMSTypeCodeToString = { { SSH, "ssh" }, { LOCALHOST, "localhost" } };

            SSubmitCmd()
                : m_nRMSTypeCode(0)
            {
            }
            void normalizeToLocal() const;
            void normalizeToRemote() const;
            size_t size() const
            {
                size_t s = (m_sSSHCfgFile.size() + 1) + sizeof(m_nRMSTypeCode);
                return s;
            }
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SSubmitCmd& val) const
            {
                return (m_sSSHCfgFile == val.m_sSSHCfgFile && m_nRMSTypeCode == val.m_nRMSTypeCode);
            }

            mutable uint16_t m_nRMSTypeCode;
            std::string m_sSSHCfgFile;
        };
        inline std::ostream& operator<<(std::ostream& _stream, const SSubmitCmd& val)
        {
            _stream << "RMS type code: " << val.m_nRMSTypeCode;
            if (val.m_nRMSTypeCode == SSubmitCmd::SSH)
                _stream << "; SSH Hosts config: " << val.m_sSSHCfgFile;

            return _stream;
        }
        inline bool operator!=(const SSubmitCmd& lhs, const SSubmitCmd& rhs)
        {
            return !(lhs == rhs);
        }
        //=============================================================================
        // A custom streamer to help boost program options to convert string options to ERmsType
        inline std::istream& operator>>(std::istream& _in, SSubmitCmd::ERmsType& _rms)
        {
            std::string token;
            _in >> token;
            if (token == "ssh")
                _rms = protocol_api::SSubmitCmd::SSH;
            else if (token == "localhost")
                _rms = protocol_api::SSubmitCmd::LOCALHOST;
            else
                throw boost::program_options::invalid_option_value(token);
            return _in;
        }

        inline std::ostream& operator<<(std::ostream& _out, SSubmitCmd::ERmsType& _rms)
        {
            switch (_rms)
            {
                case protocol_api::SSubmitCmd::SSH:
                    _out << "ssh";
                    break;
                case protocol_api::SSubmitCmd::LOCALHOST:
                    _out << "localhost";
                    break;
                case protocol_api::SSubmitCmd::UNKNOWN:
                    break;
            }
            return _out;
        }
    }
}

#endif /* defined(__DDS__SubmitCmd__) */
