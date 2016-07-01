// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_UpdateKeyCmd_h
#define DDS_UpdateKeyCmd_h
// DDS
#include "BasicCmd.h"

namespace dds
{
    namespace protocol_api
    {
        struct SUpdateKeyCmd : public SBasicCmd<SUpdateKeyCmd>
        {
            SUpdateKeyCmd();
            size_t size() const;
            void _convertFromData(const MiscCommon::BYTEVector_t& _data);
            void _convertToData(MiscCommon::BYTEVector_t* _data) const;
            bool operator==(const SUpdateKeyCmd& val) const;

            /// Helper function to set a key string in a form "propetrtyID.47598590403".
            void setKey(const std::string& _propID, uint64_t _taskID);
            /// Helper function to extract property ID from key.
            std::string getPropertyID() const;
            /// Helper function to extract task ID from key.
            uint64_t getTaskID() const;

            std::string m_sKey;
            std::string m_sValue;
        };
        std::ostream& operator<<(std::ostream& _stream, const SUpdateKeyCmd& val);
        bool operator!=(const SUpdateKeyCmd& lhs, const SUpdateKeyCmd& rhs);
    }
}
#endif
