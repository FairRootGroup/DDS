// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_DDSOptions_h
#define DDS_DDSOptions_h

namespace DDS
{
    typedef struct SDDSGeneralOptions
    {
        std::string m_workDir;                    //!< Working folder.
        unsigned int m_ddsCommanderPortRangeMin; //!< commander port range min value (should be open for incoming connection)
        unsigned int m_ddsCommanderPortRangeMax; //!< commander port range max value (should be open for incoming connection)
    } SDDSGeneralOptions_t;

    typedef struct SDDSUserDefaultOptions
    {
        SDDSGeneralOptions m_general;
    } SDDSUserDefaultsOptions_t;
}

#endif
