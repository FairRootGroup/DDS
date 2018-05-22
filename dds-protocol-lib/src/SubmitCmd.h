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
            SSubmitCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SSubmitCmd& val) const;

            std::string m_sRMSType;
            std::string m_sCfgFile;
            std::string m_sPath;
            uint32_t m_nNumberOfAgents;
        };
        std::ostream& operator<<(std::ostream& _stream, const SSubmitCmd& val);
        bool operator!=(const SSubmitCmd& lhs, const SSubmitCmd& rhs);
    } // namespace protocol_api
} // namespace dds

#endif /* defined(__DDS__SubmitCmd__) */
