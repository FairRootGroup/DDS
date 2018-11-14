// Copyright 2018 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Start.h"
#include "DDSHelper.h"
#include "InfoChannel.h"
#include "Process.h"
#include "UserDefaults.h"
#include "version.h"
// STD
#include <array>
#include <chrono>
#include <thread>
// BOOST
#include <boost/filesystem.hpp>
#include <boost/predef.h>

using namespace std;
using namespace dds::session_cmd;
using namespace dds::user_defaults_api;
using namespace MiscCommon;
namespace fs = boost::filesystem;

void CStart::start(bool _Mixed)
{
    auto start = chrono::high_resolution_clock::now();

    getNewSessionID();
    if (!checkPrecompiledWNBins(_Mixed))
    {
        if (!_Mixed)
            printHint();
        throw runtime_error("Precompiled bins check failed.");
    }
    spawnDDSCommander();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, std::milli> elapsed = end - start;
    cout << "Startup time: " << elapsed.count() << " ms\n";
}

void CStart::getNewSessionID()
{
    // Get new session ID
    string sOut;
    string sErr;
    int nExitCode(0);

    stringstream ssCmd;
    ssCmd << boost::process::search_path("dds-commander").string() << " prep-session";
    execute(ssCmd.str(), std::chrono::seconds(15), &sOut, &sErr, &nExitCode);

    if (nExitCode != 0 || !sErr.empty())
        throw runtime_error(sErr);

    m_sSessionID = sOut;

    LOG(log_stdout_clean) << "DDS session ID: " << m_sSessionID;

    // Reinit UserDefaults and Log with new session ID
    CUserDefaults::instance().reinit(boost::uuids::string_generator()(m_sSessionID),
                                     CUserDefaults::instance().currentUDFile());
    Logger::instance().reinit();

    // create workers' sandbox dir
    fs::path pathWrkSandboxDir(CUserDefaults::instance().getWrkPkgDir());
    fs::create_directories(pathWrkSandboxDir);
}

bool CStart::checkPrecompiledWNBins(bool _Mixed)
{
    // wn bin name:
    // <pakage>-<version>-<OS>-<ARCH>.tar.gz
    const string sBaseName("dds-wrk-bin");
    const string sBaseSufix(".tar.gz");
    const string sOSXArch("Darwin");

#ifdef __APPLE__
    const string sOS(sOSXArch);
#elif __linux__
    const string sOS("Linux");
#endif

    // support of x86 is dropeed, we support on 64bit
    const string sArch("amd64");

    array<string, 2> sListOS = { { "Linux", sOSXArch } };
    array<string, 1> sListArch = { { "amd64" } };

    // WN bins path
    fs::path pathWnBins(CUserDefaults::instance().getWnBinsDir());
    if (!_Mixed)
    {
        LOG(log_stdout_clean) << "Checking precompiled binaries for the local system only:";
        stringstream ssName;
        ssName << sBaseName << "-" << PROJECT_VERSION_STRING << "-" << sOS << "-"
               << (sOS == sOSXArch ? "universal" : sArch) << sBaseSufix;
        // Check availability
        fs::path pathBin(pathWnBins);
        pathBin /= ssName.str();
        bool bExists = (fs::exists(pathBin) && fs::is_regular_file(pathBin));
        LOG(log_stdout_clean) << "\t" << ssName.str() << " - " << (bExists ? "OK" : "MISSING");

        return bExists;
    }

    bool bMissing(false);
    LOG(log_stdout_clean) << "Checking precompiled binaries:";
    StringVector_t toDownload;
    for (const auto& i : sListOS)
    {
        for (const auto& j : sListArch)
        {
            stringstream ssName;
            ssName << sBaseName << "-" << PROJECT_VERSION_STRING << "-" << i << "-" << (i == sOSXArch ? "universal" : j)
                   << sBaseSufix;

            // Check availability
            fs::path pathBin(pathWnBins);
            pathBin /= ssName.str();
            bool bExists = (fs::exists(pathBin) && fs::is_regular_file(pathBin));
            if (!bExists)
            {
                bMissing = true;
                toDownload.push_back(pathBin.filename().string());
            }
            LOG(log_stdout_clean) << "\t" << ssName.str() << " - " << (bExists ? "OK" : "MISSING");
            // OSX has only one architecture
            if (i == sOSXArch)
                break;
        }
    }

    if (!toDownload.empty())
    {
        getPrecompiledWNBins(toDownload);
    }

    return (!bMissing);
}

void CStart::getPrecompiledWNBins(StringVector_t& _list)
{
    LOG(log_stdout_clean) << "One or more WNs pre-compiled binaries is missing.\n"
                          << "Looking on DDS WEB repo...";

    bool useCurl(false);
    LOG(log_stdout_clean) << "looking for CURL...";
    if (boost::process::search_path("curl").empty())
    {
        LOG(log_stdout_clean) << "- NOT FOUND";
        LOG(log_stdout_clean) << "looking for WGET...";
        if (boost::process::search_path("wget").empty())
        {
            LOG(log_stdout_clean) << "- NOT FOUND";
            throw runtime_error(
                "Can't find neither wget, nor curl commands. At least one of these commands must be available.");
        }
        else
        {
            LOG(log_stdout_clean) << "- FOUND";
        }
    }
    else
    {
        LOG(log_stdout_clean) << "- FOUND";
        useCurl = true;
    }

    stringstream ssCmd;
    if (useCurl)
        ssCmd << boost::process::search_path("curl") << " --fail -s -O";
    else
        ssCmd << boost::process::search_path("wget") << " -q";

    // Create WN bin dir
    fs::path pathWnBins(CUserDefaults::instance().getWnBinsDir());
    fs::create_directories(pathWnBins);

    // Set Working dir
    fs::path pathCur(boost::filesystem::current_path());
    fs::current_path(pathWnBins);
    // Trying to dowlond bins
    stringstream ssURL;
    ssURL << " http://dds.gsi.de/releases/add"
          << "/" << PROJECT_VERSION_STRING << "/";
    for (const auto& i : _list)
    {
        string sUrl(ssURL.str());
        sUrl += i;
        string sCmd(ssCmd.str());
        sCmd += sUrl;

        string sOut;
        string sErr;
        int nExitCode(0);
        execute(sCmd, std::chrono::seconds(30), &sOut, &sErr, &nExitCode);
        LOG(log_stdout_clean) << "\texecuting: " << sCmd;
        if (nExitCode != 0 || !sErr.empty())
        {
            LOG(log_stdout_clean) << "Error occurred while downloading precompiled binaries.";
            printHint();

            throw runtime_error(!sErr.empty() ? sErr : "Can't download pre-compiled binaries for WNs.");
            // Restore working directory
            fs::current_path(pathCur);
        }
        if (!sOut.empty())
            LOG(log_stdout_clean) << sOut;
    }
    // Restore working directory
    fs::current_path(pathCur);
}

void CStart::spawnDDSCommander()
{
    LOG(log_stdout_clean) << "Starting DDS commander...";

    string sOut;
    string sErr;
    int nExitCode(0);
    stringstream ssCmd;
    ssCmd << boost::process::search_path("dds-daemonize").string() << " \""
          << CUserDefaults::instance().getValueForKey("server.log_dir") << "\" "
          << boost::process::search_path("dds-commander").string() << " --session " << m_sSessionID << " start";

    execute(ssCmd.str(), std::chrono::seconds(30), &sOut, &sErr, &nExitCode);

    if (nExitCode != 0 || !sErr.empty())
    {
        throw runtime_error(!sErr.empty() ? sErr : "Failed to start DDS commander server.");
    }
    if (!sOut.empty())
        LOG(log_stdout_clean) << sOut;

    checkCommanderStatus();
}

void CStart::checkCommanderStatus()
{
    LOG(log_stdout_clean) << "Waiting for DDS Commander to appear online...";
    string sHost;
    string sPort;
    for (unsigned short i = 0; i < 3000; ++i)
    {
        try
        {
            findCommanderUI(&sHost, &sPort);
        }
        catch (...)
        {
        }
        if (!sHost.empty() && !sPort.empty())
            break;

        this_thread::sleep_for(chrono::milliseconds(10));
    }
    if (sHost.empty() || sPort.empty())
        throw runtime_error("Can't detect Comander's UI address. Assuming DDS Commander failed to start.");

    LOG(log_stdout_clean) << "DDS commander appears online. Testing connection...";

    boost::asio::io_service io_service;

    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(sHost, sPort);

    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

    CInfoChannel::connectionPtr_t client = CInfoChannel::makeNew(io_service, 0);
    client->connect(iterator);

    LOG(log_stdout_clean) << "DDS commander is up and running.";

    io_service.run();
}

void CStart::printHint()
{
    cout << "\nHint: DDS failed to find the worker package for the local system.\n"
         << "      You probably didn't create it.\n"
         << "      Use the wn_bin target before installing DDS: \"> make wn_bin\"\n"
         << "\n      You can also run DDS in a mixed mode with workers on Linux and on OS X in the same time.\n"
         << "      In this case DDS will take packages from the DDS binary repository or you can build them yourself.\n"
         << "      Use \"dds-session start --mixed\" to start DDS in a mixed mode." << endl;
}
