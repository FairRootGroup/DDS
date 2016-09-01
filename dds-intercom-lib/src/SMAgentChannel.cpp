// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "SMAgentChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace dds::protocol_api;
using namespace std;
using namespace dds::internal_api;

bool CSMAgentChannel::on_cmdUPDATE_KEY(SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t _attachment)
{
    LOG(debug) << "Update key message received: " << *_attachment;

    return false;
}

bool CSMAgentChannel::on_cmdDELETE_KEY(SCommandAttachmentImpl<cmdDELETE_KEY>::ptr_t _attachment)
{
    LOG(debug) << "Delete key message received: " << *_attachment;

    return false;
}

bool CSMAgentChannel::on_cmdCUSTOM_CMD(SCommandAttachmentImpl<cmdCUSTOM_CMD>::ptr_t _attachment)
{
    LOG(debug) << "Custom command message received: " << *_attachment;

    return false;
}

bool CSMAgentChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    LOG(debug) << "Simple message received: " << *_attachment;

    return false;
}