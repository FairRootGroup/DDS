//
//  SSHTunnel.cpp
//  PoD
//
//  Created by Anar Manafov on 16.02.11.
//  Copyright 2011 GSI. All rights reserved.
//
//=============================================================================
#include "SSHTunnel.h"
// API
#include <signal.h>
// STD
#include <fstream>
// MiscCommon
#include "INet.h"
#include "Process.h"
#include "SysHelper.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
CSSHTunnel::~CSSHTunnel()
{
    if (m_needToKill)
        killTunnel();
}
//=============================================================================
void CSSHTunnel::setPidFile(const string& _filename)
{
    m_pidFile = _filename;
}
//=============================================================================
pid_t CSSHTunnel::pid()
{
    ifstream f(m_pidFile.c_str());
    if (!f.is_open())
    {
        m_pid = 0;
        return m_pid;
    }
    f >> m_pid;
    return m_pid;
}
//=============================================================================
void CSSHTunnel::killTunnel()
{
    // refresh the pid of the tunnel
    pid();
    // kill the tunnel if exist
    if (0 != m_pid)
    {
        kill(m_pid, SIGKILL);
        short count(0);
        const short max_try(100); // force to wait for about 5 secs
        while (IsProcessExist(m_pid))
        {
            ++count;
            if (count >= max_try)
                kill(m_pid, SIGTERM);

            usleep(50000); // delays for 0.05 seconds
        }
        m_pid = 0;
    }

    unlink(m_pidFile.c_str());
}
//=============================================================================
void CSSHTunnel::create(const string& _connectionStr, size_t _localPort, size_t _remotePort, const string& _openDomain)
{
    // delete tunnel's file
    killTunnel();
    // create an ssh tunnel on PoD Server port
    switch (fork())
    {
        case -1:
            // Unable to fork
            throw runtime_error("Unable to create an SSH tunnel.");
        case 0:
        {
            // create SSH Tunnel
            // TODO: So far we have a hard-coded path to the script.
            // Make it optional
            string cmd("$POD_LOCATION/bin/private/ssh-tunnel");
            smart_path(&cmd);

            string pid_arg("-f");
            pid_arg += m_pidFile;

            string l_arg("-l");
            l_arg += _connectionStr;

            stringstream p_arg;
            p_arg << "-p" << _localPort;

            stringstream r_arg;
            r_arg << "-r" << _remotePort;

            string o_arg("-o");
            if (!_openDomain.empty())
                o_arg += _openDomain;
            else
                o_arg.clear();

            string i_arg("-i");
            if (!m_IdentityFile.empty())
            {
                smart_path(&m_IdentityFile);
                i_arg += m_IdentityFile;
            }
            else
                i_arg.clear();

            string sBatch;
            //                if( _opt.m_batchMode )
            //                    sBatch = "-b";

            execl(cmd.c_str(),
                  "ssh-tunnel",
                  pid_arg.c_str(),
                  l_arg.c_str(),
                  p_arg.str().c_str(),
                  r_arg.str().c_str(),
                  o_arg.c_str(),
                  i_arg.c_str(),
                  sBatch.c_str(),
                  NULL);
            // we shoud never come to this point of execution
            exit(1);
        }
    }
    // wait for tunnel to start
    short count(0);
    const short max_try(600); // force to wait for about 30 secs
    pid();
    while (0 == m_pid || !IsProcessExist(m_pid) || 0 != MiscCommon::INet::get_free_port(_localPort))
    {
        ++count;
        pid();
        if (count >= max_try)
            throw runtime_error("Can't setup SSH tunnel.");
        usleep(50000); // delays for 0.05 seconds
    }
}
//=============================================================================
