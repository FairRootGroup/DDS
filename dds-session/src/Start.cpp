// Copyright 2018 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "Start.h"
#include "DDSHelper.h"
#include "Process.h"
#include "Tools.h"
#include "UserDefaults.h"
#include "Version.h"
// STD
#include <array>
#include <chrono>
#include <thread>
// BOOST
#include <boost/filesystem.hpp>
#include <boost/predef.h>
#if __has_include(<boost/process/v1.hpp>)
#include <boost/process/v1.hpp>
namespace bp = boost::process::v1;
#else
#include <boost/process.hpp>
namespace bp = boost::process;
#endif

using namespace std;
using namespace dds::session_cmd;
using namespace dds::user_defaults_api;
using namespace dds::tools_api;
using namespace dds::misc;
namespace fs = boost::filesystem;

void CStart::start(bool _Mixed, bool _Lightweight)
{
    auto start = chrono::high_resolution_clock::now();

    getNewSessionID();
    if (!checkPrecompiledWNBins(_Mixed, _Lightweight))
    {
        if (!_Mixed && !_Lightweight)
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
    ssCmd << bp::search_path("dds-commander").string() << " prep-session";
    execute(ssCmd.str(), std::chrono::seconds(15), &sOut, &sErr, &nExitCode);

    if (nExitCode != 0 || !sErr.empty())
    {
        stringstream ss;
        if (nExitCode != 0)
            ss << "dds-commander prep-session exited with code " << nExitCode << "; ";
        if (!sErr.empty())
            ss << "error: " << sErr;

        throw runtime_error(ss.str());
    }

    m_sSessionID = sOut;

    LOG(log_stdout_clean) << "DDS session ID: " << m_sSessionID;

    // Reinit UserDefaults and Log with new session ID
    CUserDefaults::instance().reinit(boost::uuids::string_generator()(m_sSessionID),
                                     CUserDefaults::instance().currentUDFile());
    Logger::instance().reinit();

    // create workers' sandbox dir
    fs::path pathWrkSandboxDir(CUserDefaults::instance().getWrkPkgRootDir());
    fs::create_directories(pathWrkSandboxDir);
}

bool CStart::checkPrecompiledWNBins(bool _Mixed, bool _Lightweight)
{
    // If lightweight mode is enabled, skip WN package validation entirely
    if (_Lightweight)
    {
        LOG(log_stdout_clean) << "Starting DDS in lightweight mode - WN package validation skipped.";
        LOG(log_stdout_clean) << "Note: Workers must have DDS pre-installed with DDS_COMMANDER_BIN_LOCATION and "
                                 "DDS_COMMANDER_LIBS_LOCATION set.";
        return true;
    }

    // wn bin name:
    // <package>-<version>-<OS>-<ARCH>.tar.gz
    const string sBaseName("dds-wrk-bin");
    const string sBaseSufix(".tar.gz");
    const string sOSXArch("Darwin");

#ifdef __APPLE__
    const string sOS(sOSXArch);
#elif __linux__
    const string sOS("Linux");
#endif

#if BOOST_ARCH_X86
#if BOOST_ARCH_X86_64
    const string sArch("x86_64");
    // support of x86 is dropped, we support only 64bit
// #elif BOOST_ARCH_X86_32
//       const string sArch("x86_32");
#endif
#elif BOOST_ARCH_ARM
    const string sArch("arm64");
#endif

    array<string, 2> sListOS = { { "Linux", sOSXArch } };
    array<string, 1> sListArch = { { "x64" } };

    // WN bins path
    fs::path pathWnBins(CUserDefaults::instance().getWnBinsDir());
    if (!_Mixed)
    {
        LOG(log_stdout_clean) << "Checking precompiled binaries for the local system only:";
        stringstream ssName;
        ssName << sBaseName << "-" << DDS_VERSION_STRING << "-" << sOS << "-" << sArch << sBaseSufix;
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
            ssName << sBaseName << "-" << DDS_VERSION_STRING << "-" << i << "-" << (i == sOSXArch ? "universal" : j)
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
    if (bp::search_path("curl").empty())
    {
        LOG(log_stdout_clean) << "- NOT FOUND";
        LOG(log_stdout_clean) << "looking for WGET...";
        if (bp::search_path("wget").empty())
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
        ssCmd << bp::search_path("curl") << " --fail -s -O";
    else
        ssCmd << bp::search_path("wget") << " -q";

    // Create WN bin dir
    fs::path pathWnBins(CUserDefaults::instance().getWnBinsDir());
    fs::create_directories(pathWnBins);

    // Set Working dir
    fs::path pathCur(boost::filesystem::current_path());
    fs::current_path(pathWnBins);
    // Trying to download bins
    stringstream ssURL;
    ssURL << " http://dds.gsi.de/releases/add"
          << "/" << DDS_VERSION_STRING << "/";
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
    ssCmd << bp::search_path("dds-daemonize").string() << " \""
          << CUserDefaults::instance().getValueForKey("server.log_dir") << "\" "
          << bp::search_path("dds-commander").string() << " --session " << m_sSessionID << " start";

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
        throw runtime_error("Can't detect commander's UI address. Assuming DDS Commander failed to start.");

    LOG(log_stdout_clean) << "DDS commander appears online. Testing connection...";

    try
    {
        CSession session;
        session.attach(m_sSessionID);

        LOG(log_stdout_clean) << "DDS commander is up and running.";

        SCommanderInfoRequest::request_t requestInfo;
        SCommanderInfoRequest::ptr_t requestPtr = SCommanderInfoRequest::makeRequest(requestInfo);

        requestPtr->setMessageCallback(
            [](const SMessageResponseData& _message)
            {
                LOG((_message.m_severity == dds::intercom_api::EMsgSeverity::error) ? log_stderr : log_stdout)
                    << "Server reports: " << _message.m_msg;
            });

        requestPtr->setDoneCallback([&session]() { session.unblockCurrentThread(); });

        requestPtr->setResponseCallback(
            [](const SCommanderInfoResponseData& _info)
            {
                LOG(debug) << "UI agent has received pid of the commander server: " << _info.m_pid;
                LOG(log_stdout_clean) << "------------------------";
                LOG(log_stdout_clean) << "DDS commander server: " << _info.m_pid;
                LOG(log_stdout_clean) << "------------------------";
            });

        session.sendRequest<SCommanderInfoRequest>(requestPtr);
        session.blockCurrentThread();
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return;
    }
}

void CStart::printHint()
{
    cout << "\nHint: DDS failed to find the worker package for the local system.\n"
         << "      You have several options:\n"
         << "      1. Create worker packages: \"> make wn_bin\"\n"
         << "      2. Use mixed mode: \"dds-session start --mixed\"\n"
         << "      3. Use lightweight mode: Set DDS_LIGHTWEIGHT_PACKAGE=1 or use --lightweight\n"
         << "         (requires DDS pre-installed on worker nodes)" << endl;
}
