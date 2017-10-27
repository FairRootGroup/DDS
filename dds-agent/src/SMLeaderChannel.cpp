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

bool CSMLeaderChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment, SSenderInfo& _sender)
{
    return true;
}
