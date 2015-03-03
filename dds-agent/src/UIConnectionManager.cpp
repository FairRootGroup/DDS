// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#include "UIConnectionManager.h"

using namespace std;
using namespace dds;
using namespace MiscCommon;

CUIConnectionManager::CUIConnectionManager(boost::asio::io_service& _io_service,
                                           boost::asio::ip::tcp::endpoint& _endpoint)
    : CConnectionManagerImpl<CUIChannel, CUIConnectionManager>(_io_service, _endpoint)
{
}

CUIConnectionManager::~CUIConnectionManager()
{
}

void CUIConnectionManager::_createInfoFile(size_t _port) const
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

    f << "[agent]\n"
      << "host=" << srvHost << "\n"
      << "user=" << srvUser << "\n"
      << "port=" << _port << "\n" << endl;
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
}

bool CUIConnectionManager::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                                            CUIChannel::weakConnectionPtr_t _channel)
{
    LOG(debug) << "Forwarding a key notification update to commander channel: " << *_attachment;
    auto p = m_commanderChannel.lock();
    p->updateKey(_attachment->m_sKey, _attachment->m_sValue);
    return true;
}

void CUIConnectionManager::notifyAboutKeyUpdate(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    try
    {
        LOG(debug) << "Broadcasting key update notification to all connected UI channels. Attachment: " << *_attachment;
        // broadcast to all subscribers about key updates
        auto condition = [](CUIChannel::connectionPtr_t _v)
        {
            return (_v->getChannelType() == EChannelType::KEY_VALUE_GUARD);
        };
        accumulativeBroadcastMsg<cmdUPDATE_KEY>(*_attachment, condition);
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
