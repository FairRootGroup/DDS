// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__MoveFileCmd__
#define __DDS__MoveFileCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SMoveFileCmd : public SBasicCmd<SMoveFileCmd>
        {
            SMoveFileCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SMoveFileCmd& _val) const;

            std::string m_filePath;          ///< Path to the file
            std::string m_requestedFileName; ///< Requested name of the file
            uint16_t m_srcCommand;           ///< Source command which initiated file transport
        };
        std::ostream& operator<<(std::ostream& _stream, const SMoveFileCmd& _val);
        bool operator!=(const SMoveFileCmd& lhs, const SMoveFileCmd& rhs);
    }
}

#endif /* defined(__DDS__MoveFileCmd__) */
