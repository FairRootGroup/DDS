// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
//=============================================================================
// STD
#include <string>
// MiscCommon
#include "PoDUserDefaultsOptions.h"
//=============================================================================
class CPoDEnvironment
{
    public:
        CPoDEnvironment();
        ~CPoDEnvironment();

    public:
        void init();
        std::string version() const
        {
            return m_localVer;
        }
        std::string PoDPath() const
        {
            return m_PoDPath;
        }
        const PoD::SPoDUserDefaultsOptions_t getUD() const
        {
            assert( m_ud ); // did you forget to call the Init method?
            return *m_ud;
        }
        std::string srvInfoFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "etc/server_info.cfg";
            return ret;
        }
        std::string srvInfoFileRemote() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "etc/remote_server_info.cfg";
            return ret;
        }
        std::string tunnelXpdPidFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "rmt_srv_tunnel_xpd.pid";
            return ret;
        }
        std::string tunnelAgentPidFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "rmt_srv_tunnel_agent.pid";
            return ret;
        }
        std::string tunnelRemotePidFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "tunnel_pod-remote.pid";
            return ret;
        }
        std::string pod_remotePidFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "pod-remote.pid";
            return ret;
        }
        std::string xpdCfgFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "etc/xpd.cf";
            return ret;
        }
        std::string agentPidFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "pod-agent.pid";
            return ret;
        }
        std::string pipe_log_enginePipeFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += ".pod_remote_pipe";
            return ret;
        }
        std::string pod_remoteCfgFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_dotPoDPath );
            ret += "etc/pod-remote.cfg";
            return ret;
        }
        std::string pod_sshCfgFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_dotPoDPath );
            ret += "etc/pod-ssh.cfg";
            return ret;
        }

    private:
        void _localVersion();

    private:
        std::string m_PoDPath;
        std::string m_localVer;
        PoD::SPoDUserDefaultsOptions_t *m_ud;
        std::string m_wrkDir;
        std::string m_dotPoDPath;
};

#endif
