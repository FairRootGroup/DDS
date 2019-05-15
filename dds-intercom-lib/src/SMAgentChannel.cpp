// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SMAgentChannel.h"

using namespace dds;
using namespace dds::protocol_api;
using namespace std;
using namespace dds::internal_api;

CSMAgentChannel::CSMAgentChannel(boost::asio::io_context& _service,
                                 const string& _inputName,
                                 const string& _outputName,
                                 uint64_t _ProtocolHeaderID,
                                 EMQOpenType _inputOpenType,
                                 EMQOpenType _outputOpenType)
    : CBaseSMChannelImpl<CSMAgentChannel>(
          _service, _inputName, _outputName, _ProtocolHeaderID, _inputOpenType, _outputOpenType)
{
}
