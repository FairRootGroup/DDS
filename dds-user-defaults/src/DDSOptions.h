// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_DDSOptions_h
#define DDS_DDSOptions_h

namespace dds
{
    typedef struct SDDSGeneralOptions
    {
        std::string m_workDir;                   //!< Working folder.
        unsigned int m_ddsCommanderPortRangeMin; //!< commander port range min value (should be open for incoming connection)
        unsigned int m_ddsCommanderPortRangeMax; //!< commander port range max value (should be open for incoming connection)
        std::string m_logDir;                    //!< Logging directory.
        unsigned int m_logSeverityLevel;         //!< Log severity level
        unsigned int m_logRotationSize;          //!< Log rotation size in bytes
        bool m_logHasConsoleOutput;              //!< True if output log also to console
    } SDDSGeneralOptions_t;

    typedef struct SDDSUserDefaultOptions
    {
        SDDSGeneralOptions m_general;
    } SDDSUserDefaultsOptions_t;
}

#endif
