// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_DDSOptions_h
#define DDS_DDSOptions_h

#include "def.h"

namespace dds
{
    namespace user_defaults_api
    {
        typedef struct SDDSServerOptions
        {
            //!< Working folder.
            std::string m_workDir;
            std::string m_workDir_NoSID; // TODO: remove it once UD supports multiple sessions per instance
            //!< Sandbox folder. The directory is used for worker packages. In case if RMS can't access DDS' working
            // directory
            std::string m_sandboxDir;
            //!< commander port range min value (should be open for incoming connection)
            unsigned int m_ddsCommanderPortRangeMin;
            //!< commander port range max value (should be open for incoming connection)
            unsigned int m_ddsCommanderPortRangeMax;
            //!< Logging directory.
            std::string m_logDir;
            //!< Log severity level
            dds::misc::ELogSeverityLevel m_logSeverityLevel;
            //!< Log rotation size in MB
            unsigned int m_logRotationSize;
            //!< True if output log also to console
            bool m_logHasConsoleOutput;
            //!< Idle time in [s] after which process will be killed by monitoring thread
            unsigned int m_idleTime;
            //!< Defines a number of days to keep DDS sessions. Not running sessions older than the specified number of
            //!< days will be auto deleted.
            unsigned int m_dataRetention;
            //!< Health check interval in seconds
            unsigned int m_agentHealthCheckInterval; // Renamed to agent_health_check_interval
            //!< Health check timeout in seconds
            unsigned int m_agentHealthCheckTimeout; // Renamed to agent_health_check_timeout

        } SDDSGeneralOptions_t;

        typedef struct SDDSAgentOptions
        {
            //!< Working folder.
            std::string m_workDir;
            // !< This options forces the given file mode on the agent side files.
            // At the moment the access permissions are only applied to user task log files (stdout and stderr)
            // Mode can be specified with octal numbers.
            std::string m_accessPermissions;
            // !< The minimum disk space.
            // The agent will trigger a self-shutdown if the free disk space is below this threshold.
            unsigned int m_diskSpaceThreshold;
        } SDDSAgentOptions_t;

        typedef struct SDDSUserDefaultOptions
        {
            SDDSServerOptions m_server;
            SDDSAgentOptions_t m_agent;
        } SDDSUserDefaultsOptions_t;
    } // namespace user_defaults_api
} // namespace dds

#endif
