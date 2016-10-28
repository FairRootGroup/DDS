// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__ConnectionManager__
#define __DDS__ConnectionManager__
// DDS
#include "AgentChannel.h"
#include "ConnectionManagerImpl.h"
#include "KeyValueManager.h"
#include "Options.h"
#include "SSHScheduler.h"
#include "Topology.h"
#include "UIChannelInfo.h"
// STD
#include <condition_variable>
#include <mutex>

namespace dds
{
    namespace commander_cmd
    {
        class CConnectionManager : public protocol_api::CConnectionManagerImpl<CAgentChannel, CConnectionManager>,
                                   public std::enable_shared_from_this<CConnectionManager>
        {
          public:
            CConnectionManager(const SOptions_t& _options);

            ~CConnectionManager();

          public:
            void newClientCreated(CAgentChannel::connectionPtr_t _newClient);
            void _start();
            void _stop();
            void _createInfoFile(const std::vector<size_t>& _ports) const;
            void _deleteInfoFile() const;

          private:
            bool on_cmdGET_AGENTS_INFO(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_AGENTS_INFO>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdGET_LOG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_LOG>::ptr_t _attachment,
                               CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdBINARY_ATTACHMENT_RECEIVED(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdSUBMIT(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSUBMIT>::ptr_t _attachment,
                              CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdACTIVATE_AGENT(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdACTIVATE_AGENT>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdSTOP_USER_TASK(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSTOP_USER_TASK>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdTRANSPORT_TEST(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdTRANSPORT_TEST>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
                                  CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdUPDATE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment,
                                  CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdUSER_TASK_DONE(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUSER_TASK_DONE>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdGET_PROP_LIST(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_PROP_LIST>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdGET_PROP_VALUES(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_PROP_VALUES>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdSET_TOPOLOGY(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSET_TOPOLOGY>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdUPDATE_TOPOLOGY(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_TOPOLOGY>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdREPLY_ID(protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_ID>::ptr_t _attachment,
                                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdENABLE_STAT(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdENABLE_STAT>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdDISABLE_STAT(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdDISABLE_STAT>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdGET_STAT(protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_STAT>::ptr_t _attachment,
                                CAgentChannel::weakConnectionPtr_t _channel);
            bool on_cmdCUSTOM_CMD(protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment,
                                  CAgentChannel::weakConnectionPtr_t _channel);

          private:
            void activateTasks(const CSSHScheduler& _shceduler);
            void stopTasks(const CAgentChannel::weakConnectionPtrVector_t& _agents,
                           CAgentChannel::weakConnectionPtr_t _channel,
                           bool _shutdownOnComplete);
            void enableDisableStatForChannels(bool _enable);
            void _createWnPkg(bool _needInlineBashScript) const;

          private:
            CGetLogChannelInfo m_getLog;
            CTestChannelInfo m_transportTest;
            CActivateAgentsChannelInfo m_ActivateAgents;
            CStopUserTasksChannelInfo m_StopUserTasks;
            CSubmitAgentsChannelInfo m_SubmitAgents;
            topology_api::CTopology m_topo;

            // TODO: This is temporary storage only. Store this information as a part of scheduler.
            typedef std::map<uint64_t, CAgentChannel::weakConnectionPtr_t> TaskIDToAgentChannelMap_t;
            TaskIDToAgentChannelMap_t m_taskIDToAgentChannelMap;
            std::mutex m_mapMutex;

            CKeyValueManager m_keyValueManager;

            std::mutex m_stopTasksMutex;
            std::condition_variable m_stopTasksCondition;

            // Statistic on/off flag
            bool m_statEnabled;
        };
    }
}
#endif /* defined(__DDS__ConnectionManager__) */
