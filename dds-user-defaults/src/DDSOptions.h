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
        std::string m_workDir; //!< Working folder.
    } SDDSGeneralOptions_t;

    typedef struct SDDSUserDefaultOptions
    {
        SDDSGeneralOptions m_general;
    } SDDSUserDefaultsOptions_t;
}

#endif
