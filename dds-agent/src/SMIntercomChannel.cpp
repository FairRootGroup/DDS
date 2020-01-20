// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SMIntercomChannel.h"

using namespace dds;
using namespace dds::protocol_api;
using namespace std;

CSMIntercomChannel::CSMIntercomChannel(boost::asio::io_context& _service,
                                       const vector<string>& _inputNames,
                                       const string& _outputName,
                                       uint64_t _protocolHeaderID,
                                       EMQOpenType _inputOpenType,
                                       EMQOpenType _outputOpenType)
    : CBaseSMChannelImpl<CSMIntercomChannel>(
          _service, _inputNames, _outputName, _protocolHeaderID, _inputOpenType, _outputOpenType)
{
}

CSMIntercomChannel::~CSMIntercomChannel()
{
    removeMessageQueue();
}
