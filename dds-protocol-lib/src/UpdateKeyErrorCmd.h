// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_UpdateKeyErrorCmd_h
#define DDS_UpdateKeyErrorCmd_h
// DDS
#include "BasicCmd.h"
#include "UpdateKeyCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SUpdateKeyErrorCmd : public SBasicCmd<SUpdateKeyErrorCmd>
        {
            SUpdateKeyErrorCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SUpdateKeyErrorCmd& val) const;

            SUpdateKeyCmd m_serverCmd;
            SUpdateKeyCmd m_userCmd;
            uint16_t m_errorCode;
        };
        std::ostream& operator<<(std::ostream& _stream, const SUpdateKeyErrorCmd& val);
        bool operator!=(const SUpdateKeyErrorCmd& lhs, const SUpdateKeyErrorCmd& rhs);
    }
}
#endif
