// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "MoveFileCmd.h"

using namespace std;
using namespace dds;
using namespace dds::protocol_api;

SMoveFileCmd::SMoveFileCmd()
    : m_filePath()
    , m_requestedFileName()
    , m_srcCommand(0)
{
}
size_t SMoveFileCmd::size() const
{
    return dsize(m_filePath) + dsize(m_requestedFileName) + dsize(m_srcCommand);
}

bool SMoveFileCmd::operator==(const SMoveFileCmd& _val) const
{
    return (m_filePath == _val.m_filePath && m_requestedFileName == _val.m_requestedFileName &&
            m_srcCommand == _val.m_srcCommand);
}

void SMoveFileCmd::_convertFromData(const MiscCommon::BYTEVector_t& _data)
{
    SAttachmentDataProvider(_data).get(m_srcCommand).get(m_requestedFileName).get(m_filePath);
}

void SMoveFileCmd::_convertToData(MiscCommon::BYTEVector_t* _data) const
{
    SAttachmentDataProvider(_data).put(m_srcCommand).put(m_requestedFileName).put(m_filePath);
}

std::ostream& dds::protocol_api::operator<<(std::ostream& _stream, const SMoveFileCmd& _val)
{
    _stream << "filePath=" << _val.m_filePath << " requestedFileName=" << _val.m_requestedFileName;
    return _stream;
}

bool dds::protocol_api::operator!=(const SMoveFileCmd& lhs, const SMoveFileCmd& rhs)
{
    return !(lhs == rhs);
}
