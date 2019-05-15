// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SMIntercomChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::protocol_api;
using namespace std;

CSMIntercomChannel::CSMIntercomChannel(boost::asio::io_context& _service,
                                       const string& _inputName,
                                       const string& _outputName,
                                       uint64_t _protocolHeaderID,
                                       EMQOpenType _inputOpenType,
                                       EMQOpenType _outputOpenType)
    : CBaseSMChannelImpl<CSMIntercomChannel>(
          _service, _inputName, _outputName, _protocolHeaderID, _inputOpenType, _outputOpenType)
{
}

CSMIntercomChannel::~CSMIntercomChannel()
{
    removeMessageQueue();
}

bool CSMIntercomChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                                          const protocol_api::SSenderInfo& _sender)
{
    return false;
}

bool CSMIntercomChannel::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment,
                                          const protocol_api::SSenderInfo& _sender)
{
    return false;
}
