// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SMLeaderChannel.h"
#include "UserDefaults.h"
#include "version.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::protocol_api;
using namespace std;

CSMLeaderChannel::CSMLeaderChannel(boost::asio::io_context& _service,
                                   const string& _inputName,
                                   const string& _outputName,
                                   uint64_t _protocolHeaderID,
                                   EMQOpenType _inputOpenType,
                                   EMQOpenType _outputOpenType)
    : CBaseSMChannelImpl<CSMLeaderChannel>(
          _service, _inputName, _outputName, _protocolHeaderID, _inputOpenType, _outputOpenType)
{
    // Leader adds output for itself
    this->addOutput(_protocolHeaderID,
                    user_defaults_api::CUserDefaults::instance().getSMAgentInputName(),
                    EMQOpenType::OpenOrCreate);
}

CSMLeaderChannel::~CSMLeaderChannel()
{
    removeMessageQueue();
}

bool CSMLeaderChannel::on_cmdLOBBY_MEMBER_INFO(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment,
                                               const SSenderInfo& _sender)
{
    const string& name = _attachment->m_sMsg;
    LOG(info) << "New lobby member: ID=" << _sender.m_ID << ", SM=" << name;

    try
    {
        this->dispatchHandlers(EChannelEvents::OnLobbyMemberInfo, _sender, name);
        // Add output for lobby members, skipping output for itself
        if (_sender.m_ID != this->getProtocolHeaderID())
            this->addOutput(_sender.m_ID, name);
        SReplyCmd cmd = SReplyCmd("", (uint16_t)SReplyCmd::EStatusCode::OK, 0, cmdLOBBY_MEMBER_INFO);
        this->pushMsg<cmdREPLY>(cmd, _sender.m_ID, _sender.m_ID);
    }
    catch (exception& _e)
    {
        LOG(error) << "Failed to open MQ " << name << " of the new member " << _sender.m_ID << " error: " << _e.what();
    }

    return true;
}
