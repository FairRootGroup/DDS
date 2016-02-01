// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "UIConnectionManager.h"

using namespace std;
using namespace dds;
using namespace dds::agent_cmd;
using namespace dds::user_defaults_api;
using namespace dds::protocol_api;
using namespace MiscCommon;

CUIConnectionManager::CUIConnectionManager()
    : CConnectionManagerImpl<CUIChannel, CUIConnectionManager>(0, 0, false)
{
}

CUIConnectionManager::~CUIConnectionManager()
{
}

void CUIConnectionManager::_createInfoFile(const vector<size_t>& _ports) const
{
    const string sAgntCfg(CUserDefaults::instance().getAgentInfoFileLocation());
    LOG(MiscCommon::info) << "Creating the agent info file: " << sAgntCfg;
    ofstream f(sAgntCfg.c_str());
    if (!f.is_open() || !f.good())
    {
        string msg("Could not open the agent info configuration file: ");
        msg += sAgntCfg;
        throw runtime_error(msg);
    }

    string srvHost;
    MiscCommon::get_hostname(&srvHost);
    string srvUser;
    MiscCommon::get_cuser_name(&srvUser);

    if (_ports.size() > 0)
    {
        f << "[agent]\n"
          << "host=" << srvHost << "\n"
          << "user=" << srvUser << "\n"
          << "port=" << _ports[0] << "\n" << endl;
    }
}

void CUIConnectionManager::_deleteInfoFile() const
{
    const string sAgntCfg(CUserDefaults::instance().getAgentInfoFileLocation());
    if (sAgntCfg.empty())
        return;

    // TODO: check error code
    unlink(sAgntCfg.c_str());
}

void CUIConnectionManager::newClientCreated(CUIChannel::connectionPtr_t _newClient)
{
    // Subscribe on protocol messages
    function<bool(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, CUIChannel * _channel)> fUPDATE_KEY =
        [this](SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment, CUIChannel* _channel) -> bool
    {
        return this->on_cmdUPDATE_KEY(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdUPDATE_KEY>(fUPDATE_KEY);

    function<bool(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment, CUIChannel * _channel)> fCUSTOM_CMD =
        [this](SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment, CUIChannel* _channel) -> bool
    {
        return this->on_cmdCUSTOM_CMD(_attachment, getWeakPtr(_channel));
    };
    _newClient->registerMessageHandler<cmdCUSTOM_CMD>(fCUSTOM_CMD);
}

bool CUIConnectionManager::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                                            CUIChannel::weakConnectionPtr_t _channel)
{
    LOG(debug) << "Forwarding a key notification update to commander channel: " << *_attachment;
    auto p = m_commanderChannel.lock();
    p->updateKey(_attachment->m_sKey, _attachment->m_sValue);
    return true;
}

bool CUIConnectionManager::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment,
                                            CUIChannel::weakConnectionPtr_t _channel)
{
    LOG(debug) << "Forwarding a custom command to commander channel: " << *_attachment;
    auto p = m_commanderChannel.lock();
    p->pushMsg<cmdCUSTOM_CMD>(*_attachment);
    return true;
}

void CUIConnectionManager::notifyAboutKeyUpdate(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    try
    {
        LOG(debug) << "Broadcasting key update notification to all connected UI channels. Attachment: " << *_attachment;
        // broadcast to all subscribers about key updates
        // auto condition = [](CUIChannel::connectionPtr_t _v, bool& /*_stop*/)
        //{
        //    return (_v->getChannelType() == EChannelType::KEY_VALUE_GUARD);
        //};
        accumulativeBroadcastMsg<cmdUPDATE_KEY>(*_attachment); //, condition);
    }
    catch (exception& _e)
    {
        LOG(fatal) << _e.what();
    }
}

void CUIConnectionManager::notifyAboutSimpleMsg(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    try
    {
        LOG(debug) << "Broadcasting simple message to all connected UI channels. Attachment: " << *_attachment;
        broadcastMsg<cmdSIMPLE_MSG>(*_attachment);
    }
    catch (exception& _e)
    {
        LOG(fatal) << _e.what();
    }
}

void CUIConnectionManager::notifyAboutCustomCmd(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment)
{
    try
    {
        LOG(debug) << "Broadcasting custom command to all connected UI channels. Attachment: " << *_attachment;
        // auto condition = [](CUIChannel::connectionPtr_t _v, bool& /*_stop*/)
        //{
        //    return (_v->getChannelType() == EChannelType::CUSTOM_CMD_GUARD);
        //};
        broadcastMsg<cmdCUSTOM_CMD>(*_attachment); //, condition);
    }
    catch (exception& _e)
    {
        LOG(fatal) << _e.what();
    }
}
