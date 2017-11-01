// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SMUIChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::protocol_api;
using namespace std;

CSMUIChannel::CSMUIChannel(boost::asio::io_service& _service,
                           const string& _inputName,
                           const string& _outputName,
                           uint64_t _protocolHeaderID,
                           EMQOpenType _inputOpenType,
                           EMQOpenType _outputOpenType)
    : CBaseSMChannelImpl<CSMUIChannel>(
          _service, _inputName, _outputName, _protocolHeaderID, _inputOpenType, _outputOpenType)
{
}

CSMUIChannel::~CSMUIChannel()
{
    removeMessageQueue();
}

bool CSMUIChannel::on_rawMessage(CProtocolMessage::protocolMessagePtr_t _currentMsg, const SSenderInfo& _sender)
{
    return false;
}
