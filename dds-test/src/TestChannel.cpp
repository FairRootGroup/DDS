// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TestChannel.h"

using namespace MiscCommon;
using namespace dds;
using namespace std;

void CTestChannel::onHandshakeOK()
{
    pushMsg<cmdTRANSPORT_TEST>();
}

void CTestChannel::onHandshakeERR()
{
}

bool CTestChannel::on_cmdSIMPLE_MSG(SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t _attachment)
{
    LOG(log_stdout) << _attachment->m_sMsg;
    return true;
}

bool CTestChannel::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment)
{
    stop();
    return true;
}
