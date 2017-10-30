// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SMLeaderChannel.h"
#include "version.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::protocol_api;
using namespace std;

CSMLeaderChannel::CSMLeaderChannel(const string& _inputName, const string& _outputName, uint64_t _ProtocolHeaderID)
    : CBaseSMChannelImpl<CSMLeaderChannel>(_inputName, _outputName, _ProtocolHeaderID)
{
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
        this->addOutput(_sender.m_ID, name);
        this->pushMsg<cmdLOBBY_MEMBER_INFO_OK>(_sender.m_ID, _sender.m_ID);
    }
    catch (exception& _e)
    {
        LOG(error) << "Failed to open MQ " << name << " of the new member " << _sender.m_ID << " error: " << _e.what();
    }

    return true;
}
