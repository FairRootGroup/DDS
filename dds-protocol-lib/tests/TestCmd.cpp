// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "TestCmd.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;

STestCmd::STestCmd()
    : m_uint16(0)
    , m_uint32(0)
    , m_uint64(0)
    , m_string1()
    , m_string2()
    , m_string3()
    , m_string4()
    , m_vuint16()
    , m_vuint32()
    , m_vuint64()
    , m_vstring1()
    , m_vstring2()
{
}

size_t STestCmd::size() const
{
    return dsize(m_uint16) + dsize(m_uint32) + dsize(m_uint64) + dsize(m_string1) + dsize(m_string2) +
           dsize(m_string3) + dsize(m_string4) + dsize(m_vuint16) + dsize(m_vuint32) + dsize(m_vuint64) +
           dsize(m_vstring1) + dsize(m_vstring2);
}

bool STestCmd::operator==(const STestCmd& val) const
{
    return (m_uint16 == val.m_uint16 && m_uint32 == val.m_uint32 && m_uint64 == val.m_uint64 &&
            m_string1 == val.m_string1 && m_string2 == val.m_string2 && m_string3 == val.m_string3 &&
            m_string4 == val.m_string4 && m_vuint16 == val.m_vuint16 && m_vuint32 == val.m_vuint32 &&
            m_vuint64 == val.m_vuint64 && m_vstring1 == val.m_vstring1 && m_vstring2 == val.m_vstring2);
}

void STestCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data)
        .get(m_uint16)
        .get(m_uint32)
        .get(m_uint64)
        .get(m_string1)
        .get(m_string2)
        .get(m_vuint16)
        .get(m_vuint32)
        .get(m_vstring1)
        .get(m_string3)
        .get(m_vuint64)
        .get(m_vstring2)
        .get(m_string4);
}

void STestCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data)
        .put(m_uint16)
        .put(m_uint32)
        .put(m_uint64)
        .put(m_string1)
        .put(m_string2)
        .put(m_vuint16)
        .put(m_vuint32)
        .put(m_vstring1)
        .put(m_string3)
        .put(m_vuint64)
        .put(m_vstring2)
        .put(m_string4);
}

inline std::ostream& operator<<(std::ostream& _stream, const STestCmd& val)
{
    return _stream << "TestCmd";
}
inline bool operator!=(const STestCmd& lhs, const STestCmd& rhs)
{
    return !(lhs == rhs);
}
