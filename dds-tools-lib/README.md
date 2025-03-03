# DDS Tools API

The DDS Tools API provides a C++ interface for interacting with DDS (Dynamic Deployment System) sessions. It allows applications to create new DDS sessions, attach to existing ones, and send commands to the DDS commander.

## Overview

The main class of the Tools API is `dds::tools_api::CSession`. This class represents a DDS session and provides methods for creating, attaching to, and interacting with DDS sessions.

## Installation

The Tools API is part of the DDS package. To use it in your project, include the required headers and link against the DDS libraries:

```cpp
#include <dds/Tools.h>
```

## Basic Usage

### Creating a New DDS Session

```cpp
#include <dds/Tools.h>
#include <iostream>

using namespace dds::tools_api;

int main()
{
    try
    {
        CSession session;
        // Create a new DDS session
        boost::uuids::uuid sessionID = session.create();
        std::cout << "Created DDS session: " << boost::uuids::to_string(sessionID) << std::endl;
        
        // Use the session...
        
        // Shutdown the session when done
        session.shutdown();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

### Attaching to an Existing DDS Session

```cpp
#include <dds/Tools.h>
#include <iostream>

using namespace dds::tools_api;

int main()
{
    try
    {
        CSession session;
        // Attach to an existing DDS session
        session.attach("446b4183-1313-4648-99aa-4f8fae81311c");
        std::cout << "Attached to DDS session" << std::endl;
        
        // Use the session...
        
        // Detach from the session without shutting it down
        session.detach();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## Common Operations

### Submitting Agents

```cpp
#include <dds/Tools.h>
#include <chrono>
#include <iostream>

using namespace dds::tools_api;

int main()
{
    try
    {
        auto start = std::chrono::high_resolution_clock::now();
        CSession session;
        boost::uuids::uuid sessionID = session.create();
        
        // Create submit request to spawn agents
        SSubmitRequest::request_t submitInfo;
        submitInfo.m_config = "";
        submitInfo.m_rms = "localhost";
        submitInfo.m_instances = 10;
        submitInfo.m_slots = 0;
        submitInfo.m_pluginPath = "";
        SSubmitRequest::ptr_t submitRequestPtr = SSubmitRequest::makeRequest(submitInfo);
        
        // Subscribe on text messages from DDS server
        submitRequestPtr->setMessageCallback([](const SMessageResponseData& _message) {
            std::cout << "Server reports: " << _message.m_msg << std::endl;
        });
        
        // Subscribe on Done events
        submitRequestPtr->setDoneCallback([&session, &start]() {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            std::cout << "Submission took: " << elapsed.count() << " ms\n";
            session.unblockCurrentThread();
        });
        
        // Subscribe on job submission response
        submitRequestPtr->setResponseCallback([](const SSubmitResponseData& _response) {
            std::cout << "\nSubmission details:" << std::endl;
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
            
            if (!_response.m_jobIDs.empty()) {
                std::cout << "Job IDs:" << std::endl;
                for (const auto& id : _response.m_jobIDs) {
                    std::cout << "  • " << id << std::endl;
                }
            }
            
            if (_response.m_jobInfoAvailable) {
                std::cout << "Allocated nodes: " << _response.m_allocNodes << std::endl;
                
                // Convert state to string representation
                std::string stateStr;
                switch (_response.m_state) {
                    case 1: stateStr = "RUNNING"; break;
                    case 2: stateStr = "COMPLETED"; break;
                    default: stateStr = "UNKNOWN"; break;
                }
                std::cout << "State: " << stateStr << std::endl;
            } else {
                std::cout << "Warning: Job information is not fully available" << std::endl;
            }
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
        });
        
        // Send request to commander
        session.sendRequest<SSubmitRequest>(submitRequestPtr);
        
        // Block until job submission is complete
        session.blockCurrentThread();
        
        // Shutdown the session when done
        session.shutdown();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

### Retrieving Agent Information

```cpp
#include <dds/Tools.h>
#include <iostream>

using namespace dds::tools_api;

int main()
{
    try
    {
        CSession session;
        // Attach to an existing DDS session
        session.attach("446b4183-1313-4648-99aa-4f8fae81311c");
        
        SAgentInfoRequest::ptr_t agentInfoRequestPtr = SAgentInfoRequest::makeRequest();
        
        // Subscribe on text messages from DDS server
        agentInfoRequestPtr->setMessageCallback([](const SMessageResponseData& _message) {
            std::cout << "Server reports: " << _message.m_msg << std::endl;
        });
        
        // Subscribe on Done event
        agentInfoRequestPtr->setDoneCallback([&session]() {
            session.unblockCurrentThread();
        });
        
        // Subscribe on AgentInfo events
        agentInfoRequestPtr->setResponseCallback([](const SAgentInfoRequest::response_t& _info) {
            std::cout << "Agent ID: " << _info.m_agentID << std::endl;
            std::cout << "Host: " << _info.m_host << std::endl;
            std::cout << "Username: " << _info.m_username << std::endl;
            std::cout << "Group name: " << _info.m_groupName << std::endl;
            std::cout << "DDS path: " << _info.m_DDSPath << std::endl;
            std::cout << "Agent PID: " << _info.m_agentPid << std::endl;
            std::cout << "Total slots: " << _info.m_nSlots << std::endl;
            std::cout << "Idle slots: " << _info.m_nIdleSlots << std::endl;
            std::cout << "Executing slots: " << _info.m_nExecutingSlots << std::endl;
        });
        
        // Send request to commander
        session.sendRequest<SAgentInfoRequest>(agentInfoRequestPtr);
        
        // Block until agent info is received
        session.blockCurrentThread();
        
        // Detach from the session
        session.detach();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

### Activating a Topology

```cpp
#include <dds/Tools.h>
#include <iostream>

using namespace dds::tools_api;

int main()
{
    try
    {
        CSession session;
        // Attach to an existing DDS session
        session.attach("446b4183-1313-4648-99aa-4f8fae81311c");
        
        STopologyRequest::request_t topoInfo;
        topoInfo.m_topologyFile = "my_topology.xml";
        topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
        STopologyRequest::ptr_t topoRequestPtr = STopologyRequest::makeRequest(topoInfo);
        
        // Subscribe on text messages from DDS server
        topoRequestPtr->setMessageCallback([](const SMessageResponseData& _message) {
            std::cout << "Server reports: " << _message.m_msg << std::endl;
        });
        
        // Subscribe on Done event
        topoRequestPtr->setDoneCallback([&session]() {
            std::cout << "Topology activation complete" << std::endl;
            session.unblockCurrentThread();
        });
        
        // Subscribe on topology activation response
        topoRequestPtr->setResponseCallback([](const STopologyResponseData& _response) {
            std::string status = _response.m_activated ? "activated" : "stopped";
            std::cout << "Task " << status << ": " << _response.m_taskID 
                      << " on host " << _response.m_host 
                      << " at path " << _response.m_path << std::endl;
        });
        
        // Send request to commander
        session.sendRequest<STopologyRequest>(topoRequestPtr);
        
        // Block until topology activation is complete
        session.blockCurrentThread();
        
        // Detach from the session
        session.detach();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## Synchronous API

The Tools API also provides synchronous versions of the requests for simpler usage:

```cpp
#include <dds/Tools.h>
#include <iostream>
#include <chrono>

using namespace dds::tools_api;

int main()
{
    try
    {
        const std::string topoFile = "my_topology.xml";
        const std::chrono::seconds timeout(20);
        const std::chrono::milliseconds requestInterval(500);
        
        CSession session;
        boost::uuids::uuid sid = session.create();
        
        // Submit request to spawn agents
        SSubmitRequest::request_t submitInfo;
        submitInfo.m_rms = "localhost";
        submitInfo.m_instances = 10;
        session.syncSendRequest<SSubmitRequest>(submitInfo, timeout, &std::cout);
        
        // Wait for slots to become idle
        session.waitForNumSlots<CSession::EAgentState::idle>(10, timeout, requestInterval, &std::cout);
        
        // Activate topology
        STopologyRequest::request_t topoInfo;
        topoInfo.m_topologyFile = topoFile;
        topoInfo.m_updateType = STopologyRequest::request_t::EUpdateType::ACTIVATE;
        session.syncSendRequest<STopologyRequest>(topoInfo, timeout, &std::cout);
        
        // Shutdown the session when done
        session.shutdown();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## Example: Subscribing to Task Done Events

```cpp
#include <dds/Tools.h>
#include <iostream>

using namespace dds::tools_api;

int main()
{
    try
    {
        CSession session;
        // Attach to an existing DDS session
        session.attach("446b4183-1313-4648-99aa-4f8fae81311c");
        
        SOnTaskDoneRequest::request_t request;
        SOnTaskDoneRequest::ptr_t requestPtr = SOnTaskDoneRequest::makeRequest(request);
        
        int nTaskDoneCount = 0;
        requestPtr->setResponseCallback([&nTaskDoneCount](const SOnTaskDoneResponseData& _info) {
            ++nTaskDoneCount;
            std::cout << "Received onTaskDone event. TaskID: " << _info.m_taskID
                      << " ; ExitCode: " << _info.m_exitCode
                      << " ; Signal: " << _info.m_signal
                      << " ; Host: " << _info.m_host
                      << " ; WorkDir: " << _info.m_wrkDir
                      << " ; TaskPath: " << _info.m_taskPath << std::endl;
        });
        
        // Send request to commander
        session.sendRequest<SOnTaskDoneRequest>(requestPtr);
        
        // Keep session alive to receive task done events
        // In a real application, you would use session.blockCurrentThread() or 
        // implement a way to keep the program running
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        
        std::cout << "Total tasks completed: " << nTaskDoneCount << std::endl;
        
        // Detach from the session
        session.detach();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## Example: Shutting Down an Agent by ID

```cpp
#include <dds/Tools.h>
#include <iostream>
#include <chrono>

using namespace dds::tools_api;

int main()
{
    try
    {
        CSession session;
        // Attach to an existing DDS session
        session.attach("446b4183-1313-4648-99aa-4f8fae81311c");
        
        // Request to shutdown agent with ID 12345
        SAgentCommandRequest::request_t agentCmd;
        agentCmd.m_commandType = SAgentCommandRequestData::EAgentCommandType::shutDownByID;
        agentCmd.m_arg1 = 12345;
        
        // Use the synchronous API to send the command and wait for completion
        const std::chrono::seconds timeout(10);
        session.syncSendRequest<SAgentCommandRequest>(agentCmd, timeout, &std::cout);
        
        std::cout << "Agent shutdown request completed" << std::endl;
        
        // Detach from the session
        session.detach();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## DDS User Defaults

The Tools API provides access to DDS configuration values:

```cpp
#include <dds/Tools.h>
#include <iostream>

using namespace dds::tools_api;

int main()
{
    try
    {
        // Within an active session
        CSession session;
        boost::uuids::uuid sid = session.create();
        std::cout << "DDS log directory: " << session.userDefaultsGetValueForKey("server.log_dir") << std::endl;
        // Output example: $HOME/.DDS/log/sessions/b383d852-19a7-4ac5-9cbe-dc00d686d36f
        session.shutdown();
        
        // Without an active session
        CSession sessionNoAttach;
        std::cout << "DDS log directory: " << sessionNoAttach.userDefaultsGetValueForKey("server.log_dir") << std::endl;
        // Output example: $HOME/.DDS/log
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## API Reference

The key classes and structures of the DDS Tools API are:

* `CSession` - Main class for interacting with DDS sessions
* `SSubmitRequest` - Request to submit agents
* `STopologyRequest` - Request to manage topologies
* `SAgentInfoRequest` - Request agent information
* `SSlotInfoRequest` - Request slot information
* `SAgentCountRequest` - Request agent count information
* `SOnTaskDoneRequest` - Subscribe to task completion events
* `SAgentCommandRequest` - Send commands to agents
* `SCommanderInfoRequest` - Request information about the commander

Each request class has associated request_t and response_t types for handling data.

## Error Handling

The DDS Tools API uses exceptions to report errors. Wrap your code in try-catch blocks to handle potential errors:

```cpp
try
{
    CSession session;
    session.create();
    // Use the session...
}
catch (const std::exception& e)
{
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
}
```

## Best Practices

1. Always release resources by calling `shutdown()` or `detach()` when done with a session
2. Use synchronous requests for simple, one-off operations
3. Use asynchronous requests with callbacks for more complex scenarios or when receiving multiple responses
4. Handle exceptions properly to avoid program crashes
5. Subscribe to message callbacks to receive important notifications from the server