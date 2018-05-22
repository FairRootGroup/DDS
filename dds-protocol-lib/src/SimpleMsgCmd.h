// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__SimpleMsgCmd__
#define __DDS__SimpleMsgCmd__

// DDS
#include "BasicCmd.h"
#include "Logger.h"

namespace dds
{
    namespace protocol_api
    {
        struct SSimpleMsgCmd : public SBasicCmd<SSimpleMsgCmd>
        {
            SSimpleMsgCmd();
            SSimpleMsgCmd(const std::string& _msg, uint16_t _severity = MiscCommon::info, uint16_t _command = 0);
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SSimpleMsgCmd& val) const;

            uint16_t m_msgSeverity;
            uint16_t m_srcCommand;
            std::string m_sMsg;
        };
        std::ostream& operator<<(std::ostream& _stream, const SSimpleMsgCmd& val);
        bool operator!=(const SSimpleMsgCmd& lhs, const SSimpleMsgCmd& rhs);
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__SimpleMsgCmd__) */
