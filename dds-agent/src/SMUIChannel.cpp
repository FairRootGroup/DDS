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

bool CSMUIChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment,
                                    const protocol_api::SSenderInfo& _sender)
{
    return false;
}

bool CSMUIChannel::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment,
                                    const protocol_api::SSenderInfo& _sender)
{
    return false;
}
