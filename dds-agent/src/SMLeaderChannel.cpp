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
                                   const vector<string>& _inputNames,
                                   const string& _outputName,
                                   uint64_t _protocolHeaderID,
                                   EMQOpenType _inputOpenType,
                                   EMQOpenType _outputOpenType)
    : CBaseSMChannelImpl<CSMLeaderChannel>(
          _service, _inputNames, _outputName, _protocolHeaderID, _inputOpenType, _outputOpenType)
{
}

CSMLeaderChannel::~CSMLeaderChannel()
{
    removeMessageQueue();
}
