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
            MiscCommon::ELogSeverityLevel m_logSeverityLevel;
            //!< Log rotation size in MB
            unsigned int m_logRotationSize;
            //!< True if output log also to console
            bool m_logHasConsoleOutput;
            //!< Idle time in [s] after which process will be killed by monitoring thread
            unsigned int m_idleTime;

        } SDDSGeneralOptions_t;

        typedef struct SDDSAgentOptions
        {
            //!< Working folder.
            std::string m_workDir;
        } SDDSAgentOptions_t;

        typedef struct SDDSUserDefaultOptions
        {
            SDDSServerOptions m_server;
            SDDSAgentOptions_t m_agent;
        } SDDSUserDefaultsOptions_t;
    } // namespace user_defaults_api
} // namespace dds

#endif
