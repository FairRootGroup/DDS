// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__TestCmd__
#define __DDS__TestCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    struct STestCmd : public protocol_api::SBasicCmd<STestCmd>
    {
        STestCmd();
        size_t size() const;
        void _convertFromData(const dds::misc::BYTEVector_t& _data);
        void _convertToData(dds::misc::BYTEVector_t* _data) const;
        bool operator==(const STestCmd& val) const;

        uint16_t m_uint16;
        uint32_t m_uint32;
        uint64_t m_uint64;
        std::string m_string1;
        std::string m_string2;
        std::string m_string3;
        std::string m_string4;
        std::vector<uint16_t> m_vuint16;
        std::vector<uint32_t> m_vuint32;
        std::vector<uint64_t> m_vuint64;
        std::vector<std::string> m_vstring1;
        std::vector<std::string> m_vstring2;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const STestCmd& val);
    inline bool operator!=(const STestCmd& lhs, const STestCmd& rhs);
} // namespace dds

#endif /* defined(__DDS__TestCmd__) */
