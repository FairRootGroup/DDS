// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__ConnectionManager__
#define __DDS__ConnectionManager__
// DDS
#include "AgentChannel.h"
#include "ConditionEvent.h"
#include "ConnectionManagerImpl.h"
#include "Options.h"
#include "Scheduler.h"
#include "ToolsProtocol.h"
#include "TopoCore.h"
#include "UIChannelInfo.h"
// STD
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
            void on_cmdBINARY_ATTACHMENT_RECEIVED(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdBINARY_ATTACHMENT_RECEIVED>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdTRANSPORT_TEST(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdTRANSPORT_TEST>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdREPLY(const protocol_api::SSenderInfo& _sender,
                             protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY>::ptr_t _attachment,
                             CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdUPDATE_KEY(const protocol_api::SSenderInfo& _sender,
                                  protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment,
                                  CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdUSER_TASK_DONE(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUSER_TASK_DONE>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdGET_PROP_LIST(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_PROP_LIST>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdGET_PROP_VALUES(
                const protocol_api::SSenderInfo& _sender,
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdGET_PROP_VALUES>::ptr_t _attachment,
                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdREPLY_ID(const protocol_api::SSenderInfo& _sender,
                                protocol_api::SCommandAttachmentImpl<protocol_api::cmdREPLY_ID>::ptr_t _attachment,
                                CAgentChannel::weakConnectionPtr_t _channel);
            void on_cmdCUSTOM_CMD(const protocol_api::SSenderInfo& _sender,
                                  protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment,
                                  CAgentChannel::weakConnectionPtr_t _channel);

          private:
            template <protocol_api::ECmdType _cmd, class... Args>
            void broadcastUpdateTopologyAndWait(weakChannelInfo_t::container_t agents,
                                                CAgentChannel::weakConnectionPtr_t _channel,
                                                const std::string& _msg,
                                                Args&&... args);
            template <protocol_api::ECmdType _cmd>
            void broadcastUpdateTopologyAndWait_impl(size_t index, weakChannelInfo_t _agent);
            template <protocol_api::ECmdType _cmd>
            void broadcastUpdateTopologyAndWait_impl(
                size_t index,
                weakChannelInfo_t _agent,
                typename protocol_api::SCommandAttachmentImpl<_cmd>::ptr_t _attachment);
            template <protocol_api::ECmdType _cmd>
            void broadcastUpdateTopologyAndWait_impl(size_t index,
                                                     weakChannelInfo_t _agent,
                                                     const std::string& _filePath,
                                                     const std::string& _filename);
            template <protocol_api::ECmdType _cmd>
            void broadcastUpdateTopologyAndWait_impl(size_t index,
                                                     weakChannelInfo_t _agent,
                                                     const std::vector<std::string>& _filePaths,
                                                     const std::vector<std::string>& _filenames);
            template <protocol_api::ECmdType _cmd>
            void broadcastUpdateTopologyAndWait_impl(
                size_t index,
                weakChannelInfo_t _agent,
                const std::vector<typename protocol_api::SCommandAttachmentImpl<_cmd>::ptr_t>& _attachments);

            void activateTasks(const CScheduler& _scheduler, CAgentChannel::weakConnectionPtr_t _channel);
            void _createWnPkg(bool _needInlineBashScript, bool _lightweightPkg, uint32_t _nSlots) const;
            void processToolsAPIRequests(const protocol_api::SCustomCmdCmd& _cmd,
                                         CAgentChannel::weakConnectionPtr_t _channel);
            void submitAgents(const dds::tools_api::SSubmitRequestData& _submitInfo,
                              CAgentChannel::weakConnectionPtr_t _channel);
            void updateTopology(const dds::tools_api::STopologyRequestData& _topologyInfo,
                                CAgentChannel::weakConnectionPtr_t _channel);
            void getLog(const dds::tools_api::SGetLogRequestData& _getLog, CAgentChannel::weakConnectionPtr_t _channel);
            void sendToolsAPIMsg(CAgentChannel::weakConnectionPtr_t _channel,
                                 dds::tools_api::requestID_t _requestID,
                                 const std::string& _msg,
                                 intercom_api::EMsgSeverity _severity);
            void sendUICommanderInfo(const dds::tools_api::SCommanderInfoRequestData& _info,
                                     CAgentChannel::weakConnectionPtr_t _channel);
            void sendUIAgentInfo(const dds::tools_api::SAgentInfoRequestData& _info,
                                 CAgentChannel::weakConnectionPtr_t _channel);
            void sendUISlotInfo(const dds::tools_api::SSlotInfoRequestData& _info,
                                CAgentChannel::weakConnectionPtr_t _channel);
            void sendUIAgentCount(const dds::tools_api::SAgentCountRequestData& _info,
                                  CAgentChannel::weakConnectionPtr_t _channel);

            void sendCustomCommandResponse(CAgentChannel::weakConnectionPtr_t _channel, const std::string& _json) const;
            void sendDoneResponse(CAgentChannel::weakConnectionPtr_t _channel, tools_api::requestID_t _requestID) const;

          private:
            CGetLogChannelInfo m_getLog;
            CTestChannelInfo m_transportTest;
            CUpdateTopologyChannelInfo m_updateTopology;
            CSubmitAgentsChannelInfo m_SubmitAgents;
            topology_api::CTopoCore m_topo;

            // TODO: This is temporary storage only. Store this information as a part of scheduler.
            typedef std::map<uint64_t, weakChannelInfo_t> TaskIDToAgentChannelMap_t;
            TaskIDToAgentChannelMap_t m_taskIDToAgentChannelMap;
            std::mutex m_mapMutex;

            dds::misc::CConditionEvent m_updateTopoCondition;

            // ToolsAPI's onTaskDone subscribers
            typedef std::pair<CAgentChannel::weakConnectionPtr_t, dds::tools_api::SOnTaskDoneRequestData>
                onTaskDoneSubscriberInfo_t;
            typedef std::list<onTaskDoneSubscriberInfo_t> weakConnectionPtrList_t;
            weakConnectionPtrList_t m_onTaskDoneSubscribers;
            std::mutex m_mtxOnTaskDoneSubscribers;
        };
    } // namespace commander_cmd
} // namespace dds
#endif /* defined(__DDS__ConnectionManager__) */
