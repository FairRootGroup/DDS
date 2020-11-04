// Copyright 2019 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/test/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>
// DDS
#include "Process.h"
#include "Tools.h"
#include "Topology.h"
#include "UserDefaults.h"
// STD
#include <thread>

using namespace std;
using namespace dds;
using namespace dds::tools_api;
using namespace dds::topology_api;
using namespace dds::intercom_api;
namespace fs = boost::filesystem;
namespace bp = boost::process;

BOOST_AUTO_TEST_SUITE(test_dds_tools_session)

const size_t kDDSNumTestIterations = 3;
const size_t kDDSNumParallelSessions = 3;

void createDDS(vector<CSession>& _sessions)
{
    for (auto& session : _sessions)
    {
        boost::uuids::uuid sid;
        BOOST_CHECK_NO_THROW(sid = session.create());
        BOOST_CHECK(!sid.is_nil());
        // BOOST_CHECK(CSession::getDefaultSessionID() == sid);
        BOOST_CHECK(session.IsRunning());
        BOOST_CHECK_THROW(session.create(), runtime_error);
        BOOST_CHECK_THROW(session.attach(sid), runtime_error);
    }

    vector<CSession> attachedSessions(_sessions.size());
    size_t index{ 0 };
    for (auto& session : _sessions)
    {
        auto& attachedSession = attachedSessions[index];
        BOOST_CHECK_NO_THROW(attachedSession.attach(session.getSessionID()));
        BOOST_CHECK(!attachedSession.getSessionID().is_nil());
        BOOST_CHECK(attachedSession.IsRunning());
        index++;
    }

    for (auto& session : _sessions)
    {
        BOOST_CHECK_NO_THROW(session.shutdown());
        BOOST_CHECK(session.getSessionID().is_nil());
        BOOST_CHECK(!session.IsRunning());
        BOOST_CHECK_THROW(session.shutdown(), runtime_error);
    }

    for (auto& attachedSession : attachedSessions)
    {
        BOOST_CHECK(!attachedSession.IsRunning());
        BOOST_CHECK(!attachedSession.getSessionID().is_nil());
        BOOST_CHECK_NO_THROW(attachedSession.detach());
        BOOST_CHECK(attachedSession.getSessionID().is_nil());
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_create_mult)
{
    // Start and stop DDS sessions multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        vector<CSession> sessions(kDDSNumParallelSessions);
        createDDS(sessions);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_create_single)
{
    // Start and stop DDS sessions multiple times.
    // Common DDSSession instance.
    vector<CSession> sessions(kDDSNumParallelSessions);
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        createDDS(sessions);
    }
}

void checkIdleAgents(CSession& _session, size_t _numAgents)
{
    const std::chrono::seconds timeout(30);
    const std::chrono::milliseconds requestInterval(500);

    BOOST_CHECK_NO_THROW(
        _session.waitForNumAgents<CSession::EAgentState::idle>(_numAgents, timeout, requestInterval, &std::cout));

    BOOST_CHECK_THROW(_session.waitForNumAgents<CSession::EAgentState::idle>(
                          _numAgents + 1, std::chrono::seconds(2), requestInterval, &std::cout),
                      std::runtime_error);

    SAgentCountRequest::response_t agentCountInfo;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SAgentCountRequest>(
        SAgentCountRequest::request_t(), agentCountInfo, timeout, &std::cout));
    BOOST_CHECK_EQUAL(agentCountInfo.m_activeSlotsCount, _numAgents);
    BOOST_CHECK_EQUAL(agentCountInfo.m_idleSlotsCount, _numAgents);
    BOOST_CHECK_EQUAL(agentCountInfo.m_executingSlotsCount, 0);
}

size_t countStringsInDir(const fs::path& _logDir, const string& _stringToCount)
{
    // Untar DDS agent logs
    fs::path tmpPath{ fs::temp_directory_path() / fs::unique_path() };
    fs::create_directories(tmpPath);
    fs::path tarPath = bp::search_path("tar");
    fs::path findPath = bp::search_path("find");
    stringstream ssCmd;
    ssCmd << findPath.string() << " \"" << _logDir.string() << "\" -name \"*.tar.gz\" -exec " << tarPath.string()
          << " -C "
          << "\"" << tmpPath.string() << "\""
          << " -xf {} ;";
    MiscCommon::execute(ssCmd.str(), chrono::seconds(60));

    size_t counter{ 0 };
    fs::recursive_directory_iterator dir(tmpPath), end;
    while (dir != end)
    {
        if (fs::is_regular_file(dir->path()))
        {
            std::string line;
            std::ifstream infile(dir->path().string());
            while (std::getline(infile, line))
            {
                if (line.find(_stringToCount) != std::string::npos)
                    counter++;
            }
        }
        dir++;
    }

    return counter;
}

void makeRequests(CSession& _session,
                  const boost::filesystem::path& _topoPath,
                  STopologyRequest::request_t::EUpdateType _updateType,
                  const pair<size_t, size_t>& _submitAgents,
                  const pair<size_t, size_t>& _totalAgents,
                  size_t requiredCount)
{
    const std::chrono::seconds timeout(30);

    if (_submitAgents.second > 0)
    {
        SSubmitRequest::request_t submitInfo;
        submitInfo.m_rms = "localhost";
        submitInfo.m_instances = _submitAgents.first;
        submitInfo.m_slots = _submitAgents.second;
        BOOST_CHECK_NO_THROW(_session.syncSendRequest<SSubmitRequest>(submitInfo, timeout, &std::cout));
    }

    size_t numSlots = _totalAgents.first * _totalAgents.second;
    checkIdleAgents(_session, numSlots);

    STopologyRequest::request_t topoInfo;
    topoInfo.m_topologyFile = _topoPath.string();
    topoInfo.m_updateType = _updateType;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<STopologyRequest>(topoInfo, timeout, &std::cout));

    checkIdleAgents(_session, numSlots);

    SAgentInfoRequest::responseVector_t agentInfo;
    BOOST_CHECK_NO_THROW(
        _session.syncSendRequest<SAgentInfoRequest>(SAgentInfoRequest::request_t(), agentInfo, timeout, &std::cout));
    BOOST_CHECK_EQUAL(agentInfo.size(), _totalAgents.first);

    SCommanderInfoRequest::response_t commanderInfo;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SCommanderInfoRequest>(
        SCommanderInfoRequest::request_t(), commanderInfo, timeout, &std::cout));

    // Get DDS agent logs
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SGetLogRequest>(SGetLogRequest::request_t(), timeout, &std::cout));

    // Parse DDS agent logs and count the number of successfull tasks
    // const fs::path logDir{ dds::user_defaults_api::CUserDefaults::instance().getAgentLogStorageDir() };

    // TODO: FIXME: workaround for CUserDefaults::instance().getAgentLogStorageDir(). Support of multiple DDS sessions
    // is required.
    string logDirStr{ "$HOME/.DDS/sessions/" };
    logDirStr += to_string(_session.getSessionID());
    logDirStr += "/log/agents";
    MiscCommon::smart_path(&logDirStr);
    fs::path logDir{ logDirStr };

    const string stringToCount{ "Task successfully done" };
    const size_t count{ countStringsInDir(logDir, stringToCount) };
    BOOST_CHECK_EQUAL(count, requiredCount);

    // Remove DDS logs after parsing
    fs::remove_all(logDir);
}

void runDDS(vector<CSession>& _sessions)
{
    fs::path topoPath(fs::canonical(fs::path("property_test.xml")));
    fs::path upTopoPath(fs::canonical(fs::path("property_test_up.xml")));
    fs::path downTopoPath(topoPath);

    for (auto& session : _sessions)
    {
        boost::uuids::uuid sid = session.create();
        BOOST_CHECK(!sid.is_nil());
        BOOST_CHECK(session.IsRunning());
    }

    // Initital topology
    CTopology topo(topoPath.string());
    auto numAgents = topo.getRequiredNofAgents(10);
    size_t requiredCount{ numAgents.first * numAgents.second };
    for (auto& session : _sessions)
    {
        makeRequests(
            session, topoPath, STopologyRequest::request_t::EUpdateType::ACTIVATE, numAgents, numAgents, requiredCount);
    }

    //    this_thread::sleep_for(chrono::seconds(1));

    // Upscaled topology
    CTopology upTopo(upTopoPath.string());
    auto upNumAgents = upTopo.getRequiredNofAgents(10);
    requiredCount += upNumAgents.first * upNumAgents.second;
    for (auto& session : _sessions)
    {
        makeRequests(session,
                     upTopoPath,
                     STopologyRequest::request_t::EUpdateType::UPDATE,
                     numAgents,
                     upNumAgents,
                     requiredCount);
    }

    //    this_thread::sleep_for(chrono::seconds(1));

    // Downscaled topology
    CTopology downTopo(downTopoPath.string());
    auto downNumAgents = downTopo.getRequiredNofAgents(10);
    requiredCount += downNumAgents.first * downNumAgents.second;
    for (auto& session : _sessions)
    {
        makeRequests(session,
                     downTopoPath,
                     STopologyRequest::request_t::EUpdateType::UPDATE,
                     make_pair(0, 0),
                     upNumAgents,
                     requiredCount);
    }

    for (auto& session : _sessions)
    {
        session.shutdown();
        BOOST_CHECK(session.getSessionID().is_nil());
        BOOST_CHECK(!session.IsRunning());
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        vector<CSession> sessions(kDDSNumParallelSessions);
        runDDS(sessions);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    vector<CSession> sessions(kDDSNumParallelSessions);
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        runDDS(sessions);
    }
}

void runDDSInf(vector<CSession>& _sessions)
{
    const std::chrono::seconds timeout(30);
    const std::chrono::seconds sleepTime(3);

    fs::path topoPath(fs::canonical(fs::path("property_test_inf.xml")));
    fs::path upTopoPath(fs::canonical(fs::path("property_test_inf_up.xml")));
    fs::path downTopoPath(topoPath);

    for (auto& session : _sessions)
    {
        boost::uuids::uuid sid = session.create();
        BOOST_CHECK(!sid.is_nil());
        BOOST_CHECK(session.IsRunning());
    }

    const size_t numSlots(20);

    // Submit enough agent for the upscaled topology
    SSubmitRequest::request_t submitInfo;
    submitInfo.m_rms = "localhost";
    submitInfo.m_slots = numSlots;
    for (auto& session : _sessions)
    {
        BOOST_CHECK_NO_THROW(session.syncSendRequest<SSubmitRequest>(submitInfo, timeout, &std::cout));
    }

    for (auto& session : _sessions)
    {
        checkIdleAgents(session, numSlots);
    }

    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        // Activate default topology
        STopologyRequest::request_t topoInfo;
        topoInfo.m_topologyFile = topoPath.string();
        topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
        for (auto& session : _sessions)
        {
            BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(topoInfo, timeout, &std::cout));
        }
        std::this_thread::sleep_for(sleepTime);

        // Upscale topology
        STopologyRequest::request_t upTopoInfo;
        upTopoInfo.m_topologyFile = upTopoPath.string();
        upTopoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::UPDATE;
        for (auto& session : _sessions)
        {
            BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(upTopoInfo, timeout, &std::cout));
        }
        std::this_thread::sleep_for(sleepTime);

        // Downscale topology
        STopologyRequest::request_t downTopoInfo;
        downTopoInfo.m_topologyFile = downTopoPath.string();
        downTopoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::UPDATE;
        for (auto& session : _sessions)
        {
            BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(downTopoInfo, timeout, &std::cout));
        }
        std::this_thread::sleep_for(sleepTime);

        // Stop topology
        STopologyRequest::request_t stopTopoInfo;
        stopTopoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::STOP;
        for (auto& session : _sessions)
        {
            BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(stopTopoInfo, timeout, &std::cout));
        }

        for (auto& session : _sessions)
        {
            checkIdleAgents(session, numSlots);
        }
    }

    for (auto& session : _sessions)
    {
        session.shutdown();
        BOOST_CHECK(session.getSessionID().is_nil());
        BOOST_CHECK(!session.IsRunning());
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult_inf)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        vector<CSession> sessions(kDDSNumParallelSessions);
        runDDSInf(sessions);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single_inf)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    vector<CSession> sessions(kDDSNumParallelSessions);
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        runDDSInf(sessions);
    }
}

void runIntercom(vector<CSession>& _sessions, vector<CIntercomService>& _services, vector<CCustomCmd>& _customCmds)
{
    // Subscribe on Intercom events
    for (auto& service : _services)
    {
        service.subscribeOnError([](const EErrorCode _errorCode, const string& _errorMsg) {
            // Any error message fails a test
            BOOST_CHECK(false);
        });
    }

    // Subscribe on custom command events
    for (auto& customCmd : _customCmds)
    {
        customCmd.subscribe([](const string& _command, const string& _condition, uint64_t _senderId) {
            BOOST_CHECK_EQUAL(_command, "ok");
        });
    }

    // Start intercom service
    size_t count{ 0 };
    for (auto& service : _services)
    {
        string sessionID{ to_string(_sessions[count].getSessionID()) };
        service.start(sessionID);
        count++;
    }

    // Send custom commands to each task and to all DDS sessions
    const size_t numRequests{ 10 };
    for (size_t i = 0; i < numRequests; i++)
    {
        size_t count{ 0 };
        for (auto& customCmd : _customCmds)
        {
            string sessionID{ to_string(_sessions[count].getSessionID()) };
            customCmd.send(sessionID, "");
            count++;
        }
    }

    for (auto& customCmd : _customCmds)
    {
        customCmd.send("exit", "");
    }
}

void runDDSCustomCmd(vector<CSession>& _sessions)
{
    const std::chrono::seconds timeout(30);

    fs::path topoPath(fs::canonical(fs::path("custom_cmd_test.xml")));

    for (auto& session : _sessions)
    {
        boost::uuids::uuid sid = session.create();
        BOOST_CHECK(!sid.is_nil());
        BOOST_CHECK(session.IsRunning());
    }

    // Submit DDS agents
    CTopology topo(topoPath.string());
    auto numAgents = topo.getRequiredNofAgents(5);
    size_t requiredCount{ numAgents.first * numAgents.second };
    SSubmitRequest::request_t submitInfo;
    submitInfo.m_rms = "localhost";
    submitInfo.m_slots = numAgents.first * numAgents.second;
    for (auto& session : _sessions)
    {
        BOOST_CHECK_NO_THROW(session.syncSendRequest<SSubmitRequest>(submitInfo, timeout, &std::cout));
    }

    for (auto& session : _sessions)
    {
        checkIdleAgents(session, requiredCount);
    }

    // Activate default topology
    STopologyRequest::request_t topoInfo;
    topoInfo.m_topologyFile = topoPath.string();
    topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
    for (auto& session : _sessions)
    {
        BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(topoInfo, timeout, &std::cout));
    }

    // DDS Intercom services and custom commands for communication with tasks
    vector<CIntercomService> services(_sessions.size());
    vector<CCustomCmd> customCmds;
    for (auto& service : services)
    {
        customCmds.push_back(CCustomCmd(service));
    }
    runIntercom(_sessions, services, customCmds);

    // Wait until all tasks are done
    for (auto& session : _sessions)
    {
        checkIdleAgents(session, requiredCount);
    }

    for (auto& session : _sessions)
    {
        session.shutdown();
        BOOST_CHECK(session.getSessionID().is_nil());
        BOOST_CHECK(!session.IsRunning());
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult_cc)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        vector<CSession> sessions(kDDSNumParallelSessions);
        runDDSCustomCmd(sessions);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single_cc)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    vector<CSession> sessions(kDDSNumParallelSessions);
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        runDDSCustomCmd(sessions);
    }
}

void runDDSEnv(vector<CSession>& _sessions)
{
    const std::chrono::seconds timeout(30);
    const fs::path topoPath(fs::canonical(fs::path("env_test.xml")));

    for (auto& session : _sessions)
    {
        boost::uuids::uuid sid = session.create();
        BOOST_CHECK(!sid.is_nil());
        BOOST_CHECK(session.IsRunning());
    }

    // Submit DDS agents
    CTopology topo(topoPath.string());
    auto numAgents = topo.getRequiredNofAgents(9);
    size_t requiredCount{ numAgents.first * numAgents.second };
    SSubmitRequest::request_t submitInfo;
    submitInfo.m_rms = "localhost";
    submitInfo.m_slots = numAgents.first * numAgents.second;
    for (auto& session : _sessions)
    {
        BOOST_CHECK_NO_THROW(session.syncSendRequest<SSubmitRequest>(submitInfo, timeout, &std::cout));
    }

    for (auto& session : _sessions)
    {
        checkIdleAgents(session, requiredCount);
    }

    // Activate default topology
    STopologyRequest::request_t topoInfo;
    topoInfo.m_topologyFile = topoPath.string();
    topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
    for (auto& session : _sessions)
    {
        BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(topoInfo, timeout, &std::cout));
    }

    for (auto& session : _sessions)
    {
        // Get DDS agent logs
        BOOST_CHECK_NO_THROW(session.syncSendRequest<SGetLogRequest>(SGetLogRequest::request_t(), timeout, &std::cout));

        // Parse DDS agent logs and count the number of successfull tasks
        // const fs::path logDir{ dds::user_defaults_api::CUserDefaults::instance().getAgentLogStorageDir() };

        // TODO: FIXME: workaround for CUserDefaults::instance().getAgentLogStorageDir(). Support of multiple DDS
        // sessions is required.
        string logDirStr{ "$HOME/.DDS/sessions/" };
        logDirStr += to_string(session.getSessionID());
        logDirStr += "/log/agents";
        MiscCommon::smart_path(&logDirStr);
        fs::path logDir{ logDirStr };

        const string stringToCount{ "Task successfully done" };
        const size_t count{ countStringsInDir(logDir, stringToCount) };
        BOOST_CHECK_EQUAL(count, requiredCount);

        // Remove DDS logs after parsing
        fs::remove_all(logDir);
    }

    // Wait until all tasks are done
    for (auto& session : _sessions)
    {
        checkIdleAgents(session, requiredCount);
    }

    for (auto& session : _sessions)
    {
        session.shutdown();
        BOOST_CHECK(session.getSessionID().is_nil());
        BOOST_CHECK(!session.IsRunning());
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult_env)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        vector<CSession> sessions(kDDSNumParallelSessions);
        runDDSEnv(sessions);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single_env)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    vector<CSession> sessions(kDDSNumParallelSessions);
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        runDDSEnv(sessions);
    }
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_user_defaults_with_session)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    CSession session;
    boost::uuids::uuid sid = session.create();
    BOOST_CHECK(!sid.is_nil());
    BOOST_CHECK(!session.userDefaultsGetValueForKey("server.work_dir").empty());
    BOOST_CHECK(session.userDefaultsGetValueForKey("bad_key").empty());
    BOOST_CHECK(!session.userDefaultsGetValueForKey("server.log_dir")
                     .empty()); // expected value: $HOME/.DDS/log/sessions/b383d852-19a7-4ac5-9cbe-dc00d686d36f
    session.shutdown();
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_user_defaults_without_session)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    CSession session;
    BOOST_CHECK(!session.userDefaultsGetValueForKey("server.work_dir").empty());
    BOOST_CHECK(!session.userDefaultsGetValueForKey("server.log_dir").empty()); // expected value: $HOME/.DDS/log
}

BOOST_AUTO_TEST_SUITE_END()
