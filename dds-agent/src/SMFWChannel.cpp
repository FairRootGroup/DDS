// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SMFWChannel.h"

using namespace dds;
using namespace std;


CSMFWChannel::CSMFWChannel(const string& _inputName, const string& _outputName)
    : CBaseSMChannelImpl<CSMFWChannel>(_inputName, _outputName)
{
}

CSMFWChannel::~CSMFWChannel()
{
    // removeMessageQueue();
}

bool CSMFWChannel::on_rawMessage(protocol_api::CProtocolMessage::protocolMessagePtr_t _currentMsg)
{
    return false;
}
