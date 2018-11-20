// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AGENT__CCommanderChannel__
#define __DDS__AGENT__CCommanderChannel__

// DDS
#include "ClientChannelImpl.h"
#include "SMFWChannel.h"
#include "Topology.h"

namespace dds
{
    namespace agent_cmd
    {
        class CCommanderChannel : public protocol_api::CClientChannelImpl<CCommanderChannel>
        {
          public:
            CCommanderChannel(boost::asio::io_service& _service, uint64_t _ProtocolHeaderID);

          public:
            REGISTER_DEFAULT_REMOTE_ID_STRING

            RAW_MESSAGE_HANDLER(CCommanderChannel, on_rawMessage)

            CSMFWChannel::weakConnectionPtr_t getSMFWChannel();

          private:
            bool on_rawMessage(protocol_api::CProtocolMessage::protocolMessagePtr_t _currentMsg);

            uint16_t m_connectionAttempts;
            CSMFWChannel::connectionPtr_t m_SMFWChannel;
            std::map<uint64_t, uint64_t> m_taskIDToChannelIDMap;
            std::mutex m_taskIDToChannelIDMapMutex;
            topology_api::CTopology m_topo;
        };
    } // namespace agent_cmd
} // namespace dds

#endif /* defined(__DDS__AGENT__CCommanderChannel__) */
