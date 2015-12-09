// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DeleteKeyCmd__
#define __DDS__DeleteKeyCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SDeleteKeyCmd : public SBasicCmd<SDeleteKeyCmd>
        {
            SDeleteKeyCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SDeleteKeyCmd& val) const;

            std::string m_sKey;
        };
        std::ostream& operator<<(std::ostream& _stream, const SDeleteKeyCmd& val);
        bool operator!=(const SDeleteKeyCmd& lhs, const SDeleteKeyCmd& rhs);
    }
}

#endif /* defined(__DDS__DeleteKeyCmd__) */
