#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

// STD
#include <fstream>
// BOOST
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
// DDS
#include "MiscUtils.h"
#include "UserDefaults.h"

using namespace std;
using namespace dds::user_defaults_api;
namespace fs = boost::filesystem;

BOOST_AUTO_TEST_SUITE(Test_UserDefaults);

BOOST_AUTO_TEST_CASE(Test_UserDefaults_DefaultSID_FromEnv)
{
    const string sidTempalte("ed0eac3e-24d7-4cae-a46c-a1cece17d7fc");
    int ret = setenv("DDS_SESSION_ID", sidTempalte.c_str(), 1);
    BOOST_TEST(ret == 0, "Failed to set DDS_SESSION_ID");
    const string sid = CUserDefaults::instance().getDefaultSID();
    BOOST_TEST(sid == sidTempalte);
}

BOOST_AUTO_TEST_CASE(Test_UserDefaults_DefaultSID_FromSIDFile)
{
    const string sidTempalte("ed0eac3e-24d7-4cae-a46c-a1cece17d7fc");

    // Make sure DDS_SESSION_ID is not set
    unsetenv("DDS_SESSION_ID");

    /*    const string sidFile = CUserDefaults::instance().getDefaultSIDFile();

        // As we work with a real default SID file, we need to restore its value once test is finished.
        // Save current default value
        string sidDefault;
        ifstream f_bup_i(sidFile.c_str());
        if (f_bup_i.is_open())
        {
            f_bup_i >> sidDefault;
        }

        // Get the default SID
        BOOST_TEST(!sidFile.empty(), "Failed to get the name of the default SID file.");
        ofstream f(sidFile.c_str());
        BOOST_TEST(f.is_open(), "Failed to create SID file: " + sidFile);
        f << sidTempalte;
        f.close();

        const string sid = CUserDefaults::instance().getDefaultSID();
        BOOST_TEST(sid == sidTempalte);

        // Restore old default value back
        if (!sidDefault.empty())
        {
            ofstream f_bup_o(sidFile.c_str());
            f_bup_o << sidDefault;
            f_bup_o.close();
        }*/
}

BOOST_AUTO_TEST_CASE(Test_UserDefaults_CustomCfgPath)
{
    const string sidTempalte("ed0eac3e-24d7-4cae-a46c-a1cece17d7fc");
    int ret = setenv("DDS_SESSION_ID", sidTempalte.c_str(), 1);
    
    fs::path pathCfg{ fs::temp_directory_path() / fs::unique_path() };
    fs::create_directories(pathCfg);
    pathCfg /= "dds_test.cfg";

    ofstream f(pathCfg.string());
    BOOST_TEST(f.is_open());
    f << "[server]\n"
      << "idle_time=10\n";
    f.close();

    dds::misc::auto_setenv env1("DDS_CONFIG", pathCfg.string());
    CUserDefaults::instance().reinit(boost::uuids::string_generator()(CUserDefaults::instance().getDefaultSID()),
                                     CUserDefaults::instance().currentUDFile());
    fs::remove(pathCfg);
    BOOST_TEST(CUserDefaults::instance().getValueForKey("server.idle_time") == "10");
}

BOOST_AUTO_TEST_SUITE_END();
