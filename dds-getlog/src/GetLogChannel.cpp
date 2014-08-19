// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "GetLogChannel.h"
// BOOST
//#include <boost/crc.hpp>

using namespace MiscCommon;
using namespace dds;
using namespace std;

bool CGetLogChannel::on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg)
{
    m_isHandShakeOK = true;

    pushMsg<cmdGET_LOG>();

    return true;
}

bool CGetLogChannel::on_cmdBINARY_DOWNLOAD_STAT_LOG(const CProtocolMessage& _msg)
{
    SBinaryDownloadStatCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    LOG(log_stdout) << "Recieved a cmdBINARY_DOWNLOAD_STAT [" << cmd
                    << "] command from: " << socket().remote_endpoint().address().to_string();

    return true;
}

bool CGetLogChannel::on_cmdALL_LOGS_RECIEVED(const CProtocolMessage& _msg)
{
    LOG(log_stdout) << "Recieved a cmdALL_LOGS_RECIEVED command from: "
                    << socket().remote_endpoint().address().to_string() << "\n stopping and exiting...";
    stop();
    exit(EXIT_SUCCESS);
}