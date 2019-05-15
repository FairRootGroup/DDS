// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__InfoChannel__
#define __DDS__InfoChannel__
// DDS
#include "ClientChannelImpl.h"
#include "GetPropValuesCmd.h"
#include "Options.h"

namespace dds
{
    namespace info_cmd
    {
        class CInfoChannel : public protocol_api::CClientChannelImpl<CInfoChannel>
        {
            CInfoChannel(boost::asio::io_context& _service, uint64_t _protocolHeaderID = 0);

            REGISTER_DEFAULT_REMOTE_ID_STRING

          public:
            BEGIN_MSG_MAP(CInfoChannel)
                MESSAGE_HANDLER(cmdSIMPLE_MSG, on_cmdSIMPLE_MSG)
                MESSAGE_HANDLER(cmdREPLY_PID, on_cmdREPLY_PID)
                MESSAGE_HANDLER(cmdREPLY_AGENTS_INFO, on_cmdREPLY_AGENTS_INFO)
                MESSAGE_HANDLER(cmdREPLY_IDLE_AGENTS_COUNT, on_cmdREPLY_IDLE_AGENT_COUNT)
            END_MSG_MAP()

            void setOptions(const SOptions& _options)
            {
                m_options = _options;
            }

          private:
            // Message Handlers
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
                                  const protocol_api::SSenderInfo& _sender);
            bool on_cmdREPLY_PID(protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_PID>::ptr_t _attachment,
                                 const protocol_api::SSenderInfo& _sender);
            bool on_cmdREPLY_AGENTS_INFO(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_AGENTS_INFO>::ptr_t _attachment,
                const protocol_api::SSenderInfo& _sender);
            bool on_cmdREPLY_IDLE_AGENT_COUNT(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_IDLE_AGENTS_COUNT>::ptr_t _attachment,
                const protocol_api::SSenderInfo& _sender);

          private:
            SOptions m_options;
            std::mutex m_mutexCounter;
            size_t m_nCounter;
        };
    } // namespace info_cmd
} // namespace dds
#endif
