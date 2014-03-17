// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
//=============================================================================
#include "DDSSysFiles.h"
// STD
#include <sstream>
#include <stdexcept>
#include <fstream>
// MiscCommon
#include "MiscUtils.h"
#include "SysHelper.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
CPoDEnvironment::CPoDEnvironment(): m_ud( NULL )
{

}
//=============================================================================
CPoDEnvironment::~CPoDEnvironment()
{
    delete m_ud;
}
//=============================================================================
void CPoDEnvironment::init()
{
    char *pod_location;
    pod_location = getenv( "POD_LOCATION" );
    if( NULL == pod_location )
        throw runtime_error( "POD_LOCATION is not defined. Please, initialize PoD environment." );

    m_PoDPath = pod_location;
    smart_path( &m_PoDPath );
    smart_append( &m_PoDPath, '/' );

    // Read PoD User defaults
    string pathUD( PoD::showCurrentPUDFile() );
    smart_path( &pathUD );
    PoD::CPoDUserDefaults user_defaults;
    user_defaults.init( pathUD );
    m_ud = new PoD::SPoDUserDefaultsOptions_t( user_defaults.getOptions() );

    if( !m_ud )
        throw runtime_error( "PoD user defaults is not found." );

    m_wrkDir = m_ud->m_server.m_common.m_workDir;
    smart_path( &m_wrkDir );
    smart_append( &m_wrkDir, '/' );

    m_dotPoDPath = "$HOME/.PoD/";
    smart_path( &m_dotPoDPath );

    _localVersion();
}
//=============================================================================
void CPoDEnvironment::_localVersion()
{
    string version_file_name( m_PoDPath );
    version_file_name += "etc/version";
    ifstream f( version_file_name.c_str() );
    if( !f.is_open() )
        throw runtime_error( "Can't open PoD version file."
                             " Probably PoD installation is broken."
                             " You may want to reinstall PoD to repair the installation." );
    f >> m_localVer;
}
