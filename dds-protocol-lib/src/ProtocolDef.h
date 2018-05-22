// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__ProtocolDef__
#define __DDS__ProtocolDef__

// DDS

namespace dds
{
    namespace protocol_api
    {
        // Channel types
        enum EChannelType
        {
            UNKNOWN = 0,
            AGENT,
            UI,
            API_GUARD
        };
        typedef std::vector<EChannelType> channelTypeVector_t;
        const std::array<std::string, 5> gChannelTypeName{
            { "unknown", "agent", "ui", "key_value_guard", "custom_command_guard" }
        };
    } // namespace protocol_api
} // namespace dds
#endif /* __DDS__ProtocolDef__ */
