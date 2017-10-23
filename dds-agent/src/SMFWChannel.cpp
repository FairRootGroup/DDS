// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SMFWChannel.h"

using namespace dds;
using namespace std;
using namespace dds::protocol_api;

CSMFWChannel::CSMFWChannel(const string& _inputName, const string& _outputName, uint64_t _ProtocolHeaderID)
    : CBaseSMChannelImpl<CSMFWChannel>(_inputName, _outputName, _ProtocolHeaderID)
{
}

CSMFWChannel::~CSMFWChannel()
{
    // removeMessageQueue();
}

bool CSMFWChannel::on_rawMessage(CProtocolMessage::protocolMessagePtr_t _currentMsg, const SSenderInfo& _sender)
{
    return false;
}
