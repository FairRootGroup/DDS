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

CSMUIChannel::CSMUIChannel(const string& _inputName, const string& _outputName, uint64_t _ProtocolHeaderID)
    : CBaseSMChannelImpl<CSMUIChannel>(_inputName, _outputName, _ProtocolHeaderID)
{
}

CSMUIChannel::~CSMUIChannel()
{
    removeMessageQueue();
}

bool CSMUIChannel::on_rawMessage(protocol_api::CProtocolMessage::protocolMessagePtr_t _currentMsg,
                                 const protocol_api::SSenderInfo& _sender)
{
    return false;
}
