// Copyright 2014 GSI, Inc. All rights reserved.
//
// A resources file.
//
#ifndef RES_H
#define RES_H

#include "def.h"

namespace MiscCommon
{

    //---------------------- CLog -------------------------------------------
    const LPCSTR g_cszLOG_DATETIME_FRMT("%Y-%m-%d %H:%M:%S"); //!< Log Date/Time format
    const size_t LOG_DATETIME_BUFF_LEN(25);
    const LPCSTR g_cszLOG_SEVERITY_INFO("INF");
    const LPCSTR g_cszLOG_SEVERITY_WARNING("WRN");
    const LPCSTR g_cszLOG_SEVERITY_FAULT("ERR");
    const LPCSTR g_cszLOG_SEVERITY_CRITICAL_ERROR("FLT");
    const LPCSTR g_cszLOG_SEVERITY_DEBUG("DBG");

    const LPCSTR g_cszMODULENAME_CORE("CORE");
    
    //---------------------- strings -------------------------------------------
    const LPCSTR g_cszReportBugsAddr("Report bugs/comments to fairroot@gsi.de");
};
#endif
