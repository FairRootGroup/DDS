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
#include <boost/test/tools/output_test_stream.hpp>
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
using namespace dds::misc;
namespace fs = boost::filesystem;
namespace bp = boost::process;

BOOST_AUTO_TEST_SUITE(test_dds_tools_session)

const size_t kDDSNumTestIterations{ 3 };
const size_t kDDSNumParallelSessions{ 3 };
const std::chrono::seconds kTimeout{ 30 };

//
// Common functions
//

template <typename F>
void runTestMultiple(F _f)
{
    // Start and stop DDS session multiple times.
    // Each time create new DDSSession instance.
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        vector<CSession> sessions(kDDSNumParallelSessions);
        _f(sessions);
    }
}

template <typename F>
void runTestSingle(F _f)
{
    // Start and stop DDS session multiple times.
    // Common DDSSession instance.
    vector<CSession> sessions(kDDSNumParallelSessions);
    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        _f(sessions);
    }
}

size_t countStringsInDir(const fs::path& _logDir, const string& _stringToCount)
{
    // Untar DDS agent logs
    const fs::path tmpPath{ fs::temp_directory_path() / fs::unique_path() };
    fs::create_directories(tmpPath);
    const fs::path tarPath{ bp::search_path("tar") };
    const fs::path findPath{ bp::search_path("find") };
    stringstream ssCmd;
    ssCmd << findPath.string() << " \"" << _logDir.string() << "\" -name \"*.tar.gz\" -exec " << tarPath.string()
          << " -C "
          << "\"" << tmpPath.string() << "\""
          << " -xf {} ;";
    execute(ssCmd.str(), chrono::seconds(60));

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

void parseLogs(CSession& _session, const string& _stringToCount, size_t _requiredCount)
{
    // Get DDS agent logs
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SGetLogRequest>(SGetLogRequest::request_t(), kTimeout, &std::cout));

    // Parse DDS agent logs and count the number okTimeoutssfull tasks
    // const fs::path logDir{ dds::user_defaults_api::CUserDefaults::instance().getAgentLogStorageDir() };

    // FIXME: workaround for CUserDefaults::instance().getAgentLogStorageDir(). Support of multiple DDS sessions
    // is required.
    string logDirStr{ "$HOME/.DDS/sessions/" };
    logDirStr += to_string(_session.getSessionID());
    logDirStr += "/log/agents";
    smart_path(&logDirStr);
    fs::path logDir{ logDirStr };

    const size_t count{ countStringsInDir(logDir, _stringToCount) };
    BOOST_CHECK_EQUAL(count, _requiredCount);

    // Remove DDS logs after parsing
    fs::remove_all(logDir);
}

void parseLogs(vector<CSession>& _sessions, const string& _stringToCount, size_t _requiredCount)
{
    for (auto& session : _sessions)
    {
        parseLogs(session, _stringToCount, _requiredCount);
    }
}

void createSessions(vector<CSession>& _sessions)
{
    for (auto& session : _sessions)
    {
        boost::uuids::uuid sid;
        BOOST_CHECK_NO_THROW(sid = session.create());
        BOOST_CHECK(!sid.is_nil());
        BOOST_CHECK(session.IsRunning());
        BOOST_CHECK_THROW(session.create(), runtime_error);
        BOOST_CHECK_THROW(session.attach(sid), runtime_error);
    }
}

void shutdownSessions(vector<CSession>& _sessions)
{
    for (auto& session : _sessions)
    {
        BOOST_CHECK_NO_THROW(session.shutdown());
        BOOST_CHECK(session.getSessionID().is_nil());
        BOOST_CHECK(!session.IsRunning());
        BOOST_CHECK_THROW(session.shutdown(), runtime_error);
    }
}

void submitAgents(CSession& _session, uint32_t _numSlots = 0, uint32_t _numInstances = 0)
{
    SSubmitRequest::request_t submitInfo;
    submitInfo.m_rms = "localhost";
    submitInfo.m_slots = _numSlots;
    submitInfo.m_instances = _numInstances;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SSubmitRequest>(submitInfo, kTimeout, &std::cout));
}

void submitAgents(vector<CSession>& _sessions, uint32_t _numSlots = 0, uint32_t _numInstances = 0)
{
    for (auto& session : _sessions)
    {
        submitAgents(session, _numSlots, _numInstances);
    }
}

void checkIdleAgents(CSession& _session, size_t _numAgents)
{
    const std::chrono::milliseconds requestInterval(500);
    BOOST_CHECK_NO_THROW(
        _session.waitForNumAgents<CSession::EAgentState::idle>(_numAgents, kTimeout, requestInterval, &std::cout));

    BOOST_CHECK_THROW(_session.waitForNumAgents<CSession::EAgentState::idle>(
                          _numAgents + 1, std::chrono::seconds(2), requestInterval, &std::cout),
                      std::runtime_error);

    SAgentCountRequest::response_t agentCountInfo;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SAgentCountRequest>(
        SAgentCountRequest::request_t(), agentCountInfo, kTimeout, &std::cout));
    BOOST_CHECK_EQUAL(agentCountInfo.m_activeSlotsCount, _numAgents);
    BOOST_CHECK_EQUAL(agentCountInfo.m_idleSlotsCount, _numAgents);
    BOOST_CHECK_EQUAL(agentCountInfo.m_executingSlotsCount, 0);
}

void checkIdleAgents(vector<CSession>& _sessions, size_t _numAgents)
{
    for (auto& session : _sessions)
    {
        checkIdleAgents(session, _numAgents);
    }
}

void updateTopology(CSession& _session,
                    const string& _topologyFile,
                    STopologyRequest::request_t::EUpdateType _updateType)
{
    STopologyRequest::request_t topoInfo;
    topoInfo.m_topologyFile = _topologyFile;
    topoInfo.m_updateType = _updateType;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<STopologyRequest>(topoInfo, kTimeout, &std::cout));
}

void updateTopology(vector<CSession>& _sessions,
                    const string& _topologyFile,
                    STopologyRequest::request_t::EUpdateType _updateType)
{
    for (auto& session : _sessions)
    {
        updateTopology(session, _topologyFile, _updateType);
    }
}

//
// Tests
//

void createDDS(vector<CSession>& _sessions)
{
    createSessions(_sessions);

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

    shutdownSessions(_sessions);

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
    runTestMultiple(createDDS);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_create_single)
{
    runTestSingle(createDDS);
}

void makeRequests(CSession& _session,
                  const boost::filesystem::path& _topoPath,
                  STopologyRequest::request_t::EUpdateType _updateType,
                  const pair<size_t, size_t>& _submitAgents,
                  const pair<size_t, size_t>& _totalAgents,
                  size_t _requiredCount)
{
    if (_submitAgents.second > 0)
    {
        submitAgents(_session, _submitAgents.second, _submitAgents.first);
    }

    size_t numSlots = _totalAgents.first * _totalAgents.second;
    checkIdleAgents(_session, numSlots);

    updateTopology(_session, _topoPath.string(), _updateType);

    checkIdleAgents(_session, numSlots);

    SAgentInfoRequest::responseVector_t agentInfo;
    BOOST_CHECK_NO_THROW(
        _session.syncSendRequest<SAgentInfoRequest>(SAgentInfoRequest::request_t(), agentInfo, kTimeout, &std::cout));
    BOOST_CHECK_EQUAL(agentInfo.size(), _totalAgents.first);

    SCommanderInfoRequest::response_t commanderInfo;
    BOOST_CHECK_NO_THROW(_session.syncSendRequest<SCommanderInfoRequest>(
        SCommanderInfoRequest::request_t(), commanderInfo, kTimeout, &std::cout));

    parseLogs(_session, "Task successfully done", _requiredCount);
}

void runDDS(vector<CSession>& _sessions)
{
    const fs::path topoPath(fs::canonical(fs::path("property_test.xml")));
    const fs::path upTopoPath(fs::canonical(fs::path("property_test_up.xml")));
    const fs::path downTopoPath(topoPath);

    createSessions(_sessions);

    // Initital topology
    CTopology topo(topoPath.string());
    auto numAgents = topo.getRequiredNofAgents(10);
    size_t requiredCount{ numAgents.first * numAgents.second };
    for (auto& session : _sessions)
    {
        makeRequests(
            session, topoPath, STopologyRequest::request_t::EUpdateType::ACTIVATE, numAgents, numAgents, requiredCount);
    }

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

    shutdownSessions(_sessions);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult)
{
    runTestMultiple(runDDS);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single)
{
    runTestSingle(runDDS);
}

void runDDSInf(vector<CSession>& _sessions)
{
    const std::chrono::seconds sleepTime(3);
    const fs::path topoPath(fs::canonical(fs::path("property_test_inf.xml")));
    const fs::path upTopoPath(fs::canonical(fs::path("property_test_inf_up.xml")));
    const fs::path downTopoPath(topoPath);

    createSessions(_sessions);

    const size_t numSlots(20);

    // Submit enough agent for the upscaled topology
    submitAgents(_sessions, numSlots);
    checkIdleAgents(_sessions, numSlots);

    for (size_t i = 0; i < kDDSNumTestIterations; i++)
    {
        // Activate default topology
        updateTopology(_sessions, topoPath.string(), STopologyRequest::request_t::EUpdateType::ACTIVATE);
        std::this_thread::sleep_for(sleepTime);

        // Upscale topology
        updateTopology(_sessions, upTopoPath.string(), STopologyRequest::request_t::EUpdateType::UPDATE);
        std::this_thread::sleep_for(sleepTime);

        // Downscale topology
        updateTopology(_sessions, downTopoPath.string(), STopologyRequest::request_t::EUpdateType::UPDATE);
        std::this_thread::sleep_for(sleepTime);

        // Stop topology
        updateTopology(_sessions, "", STopologyRequest::request_t::EUpdateType::STOP);

        checkIdleAgents(_sessions, numSlots);
    }

    shutdownSessions(_sessions);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult_inf)
{
    runTestMultiple(runDDSInf);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single_inf)
{
    runTestSingle(runDDSInf);
}

void runIntercom(vector<CSession>& _sessions, vector<CIntercomService>& _services, vector<CCustomCmd>& _customCmds)
{
    // Subscribe on Intercom events
    for (auto& service : _services)
    {
        service.subscribeOnError(
            [](const EErrorCode /*_errorCode*/, const string& /*_errorMsg*/)
            {
                // Any error message fails a test
                BOOST_CHECK(false);
            });
    }

    // Subscribe on custom command events
    for (auto& customCmd : _customCmds)
    {
        customCmd.subscribe([](const string& _command, const string& /*_condition*/, uint64_t /*_senderId*/)
                            { BOOST_CHECK_EQUAL(_command, "ok"); });
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
    const fs::path topoPath(fs::canonical(fs::path("custom_cmd_test.xml")));

    createSessions(_sessions);

    // Submit DDS agents
    CTopology topo(topoPath.string());
    auto numAgents = topo.getRequiredNofAgents(5);
    size_t requiredCount{ numAgents.first * numAgents.second };
    submitAgents(_sessions, requiredCount);
    checkIdleAgents(_sessions, requiredCount);

    // Activate default topology
    updateTopology(_sessions, topoPath.string(), STopologyRequest::request_t::EUpdateType::ACTIVATE);

    // DDS Intercom services and custom commands for communication with tasks
    vector<CIntercomService> services(_sessions.size());
    vector<CCustomCmd> customCmds;
    for (auto& service : services)
    {
        customCmds.push_back(CCustomCmd(service));
    }
    runIntercom(_sessions, services, customCmds);

    // Wait until all tasks are done
    checkIdleAgents(_sessions, requiredCount);

    shutdownSessions(_sessions);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult_cc)
{
    runTestMultiple(runDDSCustomCmd);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single_cc)
{
    runTestSingle(runDDSCustomCmd);
}

void runDDSEnv(vector<CSession>& _sessions)
{
    const fs::path topoPath(fs::canonical(fs::path("env_test.xml")));

    createSessions(_sessions);

    // Submit DDS agents
    CTopology topo(topoPath.string());
    auto numAgents = topo.getRequiredNofAgents(9);
    size_t requiredCount{ numAgents.first * numAgents.second };
    submitAgents(_sessions, numAgents.first * numAgents.second);
    checkIdleAgents(_sessions, requiredCount);

    // Activate default topology
    updateTopology(_sessions, topoPath.string(), STopologyRequest::request_t::EUpdateType::ACTIVATE);

    // Wait until all tasks are done
    checkIdleAgents(_sessions, requiredCount);

    // Parse logs
    parseLogs(_sessions, "Task successfully done", requiredCount);

    shutdownSessions(_sessions);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_mult_env)
{
    runTestMultiple(runDDSEnv);
}

BOOST_AUTO_TEST_CASE(test_dds_tools_session_run_single_env)
{
    runTestSingle(runDDSEnv);
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

BOOST_AUTO_TEST_CASE(test_dds_tools_onTaskDone)
{
    const std::chrono::seconds sleepTime(3);
    const int tasksCount{ 9 };

    CSession session;
    boost::uuids::uuid sid = session.create();
    BOOST_CHECK(!sid.is_nil());

    // test onTaskDone events
    // Subscrube on events
    SOnTaskDoneRequest::request_t request;
    SOnTaskDoneRequest::ptr_t requestPtr = SOnTaskDoneRequest::makeRequest(request);
    int nTaskDoneCount{ 0 };
    requestPtr->setResponseCallback(
        [&nTaskDoneCount](const SOnTaskDoneResponseData& _info)
        {
            ++nTaskDoneCount;
            BOOST_TEST_MESSAGE("Recieved onTaskDone event. TaskID: " << _info.m_taskID
                                                                     << " ; ExitCode: " << _info.m_exitCode
                                                                     << " ; Signal: " << _info.m_signal);
        });
    BOOST_CHECK_NO_THROW(session.sendRequest<SOnTaskDoneRequest>(requestPtr));

    // Submit DDS agents
    const fs::path topoPath(fs::canonical(fs::path("sleep_test.xml")));
    CTopology topo(topoPath.string());
    auto numAgents = topo.getRequiredNofAgents(tasksCount);
    size_t requiredCount{ numAgents.first * numAgents.second };

    SSubmitRequest::request_t submitInfo;
    submitInfo.m_rms = "localhost";
    submitInfo.m_slots = requiredCount;
    submitInfo.m_instances = 0;
    BOOST_CHECK_NO_THROW(session.syncSendRequest<SSubmitRequest>(submitInfo, kTimeout, &std::cout));

    std::this_thread::sleep_for(sleepTime);
    STopologyRequest::request_t topoInfo;
    topoInfo.m_topologyFile = topoPath.string();
    topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
    BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(topoInfo, kTimeout, &std::cout));

    std::this_thread::sleep_for(sleepTime);
    topoInfo.m_topologyFile = "";
    topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::STOP;
    BOOST_CHECK_NO_THROW(session.syncSendRequest<STopologyRequest>(topoInfo, kTimeout, &std::cout));

    std::this_thread::sleep_for(sleepTime * 2);
    BOOST_CHECK(nTaskDoneCount == tasksCount);

    session.shutdown();
}

BOOST_AUTO_TEST_SUITE_END()
