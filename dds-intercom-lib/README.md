# DDS Intercom Library

The DDS Intercom Library provides high-level C++ APIs for distributed task communication in HPC environments. It abstracts the underlying [DDS protocol](../docs/protocol-high-level.md) complexity and provides intuitive interfaces for custom commands, key-value properties, and service coordination.

## Quick Start

```cpp
#include "Intercom.h"
using namespace dds::intercom_api;

int main() {
    // Initialize DDS communication service
    CIntercomService service;
    
    // Custom commands for task coordination
    CCustomCmd customCmd(service);
    customCmd.subscribe([](const std::string& command, const std::string& condition, uint64_t senderId) {
        if (command == "start_processing") {
            // Begin work
        }
    });
    
    // Key-value store for property sharing
    CKeyValue keyValue(service);
    keyValue.putValue("task_status", "ready");
    keyValue.subscribe([](const std::string& key, const std::string& value, uint64_t senderTaskId) {
        // Handle property updates
    });
    
    // Start service and block until completion
    service.start();
    return 0;
}
```

## Core Components

### CIntercomService

The central service that manages connections to DDS infrastructure:

- **Automatic Discovery**: Locates DDS commander via configuration files
- **Transport Selection**: Chooses optimal transport (TCP/shared memory)
- **Reconnection**: Handles connection failures with automatic retry
- **Error Reporting**: Provides detailed error information

```cpp
CIntercomService service;

// Handle service errors
service.subscribeOnError([](EErrorCode errorCode, const std::string& errorMsg) {
    switch (errorCode) {
        case EErrorCode::ConnectionFailed:
            // Network connectivity issues
            break;
        case EErrorCode::TransportServiceFailed:  
            // DDS services not running
            break;
        case EErrorCode::SendKeyValueFailed:
            // Property update failed
            break;
    }
});

service.start("session-id");  // Start with DDS session
```

### CCustomCmd - Command Communication

Enables arbitrary command exchange between distributed tasks:

**Features:**

- User-defined command strings
- Conditional delivery based on task properties
- Bidirectional communication with replies
- Timestamp tracking for message ordering

```cpp
CCustomCmd customCmd(service);

// Send commands
customCmd.send("start_processing", "role==worker");
customCmd.send("get_status", "node_id==5");

// Receive commands
customCmd.subscribe([&](const std::string& command, const std::string& condition, uint64_t senderId) {
    if (command == "get_status") {
        // Send reply back to sender
        customCmd.send("status_ok", std::to_string(senderId));
    }
});

// Handle replies
customCmd.subscribeOnReply([](const std::string& reply) {
    LOG(info) << "Received reply: " << reply;
});
```

### CKeyValue - Distributed Properties

Provides a distributed key-value store for sharing configuration and state:

**Features:**
- String-based keys and values
- Automatic propagation across all tasks
- Event-driven updates via callbacks
- Task ID tracking for update sources

```cpp
CKeyValue keyValue(service);

// Set properties
keyValue.putValue("experiment_config", "/path/to/config.json");
keyValue.putValue("processing_stage", "initialization");
keyValue.putValue("node_capacity", "8_cores");

// Monitor property changes
keyValue.subscribe([](const std::string& key, const std::string& value, uint64_t senderTaskId) {
    LOG(info) << "Property updated: " << key << " = " << value 
              << " by task " << senderTaskId;
              
    if (key == "processing_stage" && value == "ready") {
        beginProcessing();
    }
});
```

## Communication Patterns

### Master-Worker Coordination

```cpp
// Master task distributes work and collects results
void masterTask() {
    CIntercomService service;
    CCustomCmd cmd(service);
    CKeyValue props(service);
    
    // Distribute configuration
    props.putValue("work_config", serializeWorkConfig());
    
    // Signal workers to start
    cmd.send("start_work", "role==worker");
    
    // Collect results
    int completedWorkers = 0;
    props.subscribe([&](const std::string& key, const std::string& value, uint64_t sender) {
        if (key.find("result_") == 0) {
            processResult(value);
            if (++completedWorkers == totalWorkers) {
                cmd.send("shutdown", "role==worker");
            }
        }
    });
    
    service.start();
}

// Worker task processes assigned work
void workerTask() {
    CIntercomService service;
    CCustomCmd cmd(service);
    CKeyValue props(service);
    
    // Wait for work assignment
    cmd.subscribe([&](const std::string& command, const std::string& condition, uint64_t sender) {
        if (command == "start_work") {
            // Get configuration and process
            auto config = getProperty("work_config");
            auto result = processWork(config);
            props.putValue("result_" + std::to_string(getTaskId()), result);
        } else if (command == "shutdown") {
            service.stop();
        }
    });
    
    service.start();
}
```

### Pipeline Processing

```cpp
class PipelineStage {
    CIntercomService m_service;
    CKeyValue m_props;
    std::string m_stageName;
    
public:
    PipelineStage(const std::string& stageName) 
        : m_props(m_service), m_stageName(stageName) {
        
        // Process input from previous stage
        m_props.subscribe([this](const std::string& key, const std::string& value, uint64_t sender) {
            if (key == m_stageName + "_input") {
                // Process data
                auto result = transformData(value);
                
                // Send to next stage
                m_props.putValue(getNextStage() + "_input", result);
            }
        });
    }
    
    void start() { m_service.start(); }
};

// Usage
PipelineStage stage1("preprocess");
PipelineStage stage2("analyze");  
PipelineStage stage3("output");

// Start all stages concurrently
std::thread t1([&]{ stage1.start(); });
std::thread t2([&]{ stage2.start(); });
std::thread t3([&]{ stage3.start(); });
```

### Event-Driven Coordination

```cpp
class EventCoordinator {
    CIntercomService m_service;
    CCustomCmd m_cmd;
    CKeyValue m_props;
    
public:
    EventCoordinator() : m_cmd(m_service), m_props(m_service) {
        setupEventHandlers();
    }
    
private:
    void setupEventHandlers() {
        // Handle external triggers
        m_cmd.subscribe([this](const std::string& cmd, const std::string& condition, uint64_t sender) {
            if (cmd == "data_available") {
                triggerProcessing();
            } else if (cmd == "emergency_stop") {
                emergencyShutdown();
            }
        });
        
        // Monitor system health
        m_props.subscribe([this](const std::string& key, const std::string& value, uint64_t sender) {
            if (key == "cpu_usage" && std::stof(value) > 90.0) {
                m_cmd.send("reduce_load", "role==processor");
            }
        });
    }
};
```

## RMS Plugin Support

For Resource Management System integration, the library provides `CRMSPluginProtocol`:

```cpp
#include "Intercom.h"
using namespace dds::intercom_api;

// SLURM plugin example
int main(int argc, char* argv[]) {
    std::string sessionId = argv[1];
    CRMSPluginProtocol protocol(sessionId);
    
    // Handle submit requests from DDS commander
    protocol.onSubmit([&](const SSubmit& submit) {
        // Generate SLURM job script
        std::string jobScript = generateSlurmScript(submit);
        
        // Submit to SLURM scheduler
        int jobId = system(("sbatch " + jobScript).c_str());
        
        if (jobId > 0) {
            protocol.sendMessage(EMsgSeverity::info, 
                               "Job submitted with ID: " + std::to_string(jobId));
        } else {
            protocol.sendMessage(EMsgSeverity::error, "Job submission failed");
        }
    });
    
    // Handle messages from DDS
    protocol.onMessage([](const SMessage& message) {
        LOG(info) << "Received message: " << message.m_msg;
    });
    
    // Start and block until completion
    protocol.start(true);
    return 0;
}
```

## API Limits and Constraints

### Service Configuration

**Connection Constraints:**

- Automatic service discovery via DDS configuration files
- Reconnection timeout: **2 minutes maximum** (inherited from protocol layer)
- No manual transport selection (automatic TCP/shared memory choice)
- Requires active DDS session for operation

### Custom Command Limits

**Message Constraints:**

- Command strings: Limited to 2^16 (65,536) characters
- Condition strings: Limited to 2^16 (65,536) characters
- No built-in command validation or filtering
- Timestamp automatically generated (milliseconds since epoch)

### Key-Value Store Constraints

**Property Limitations:**

- Property names: Limited to 2^16 (65,536) characters
- Property values: Limited to 2^16 (65,536) characters
- No built-in data type validation (all values stored as strings)
- Memory-only storage (no persistence)
- No automatic cleanup or expiration

### Error Handling Constraints

**Error Reporting:**

- Three error types: `ConnectionFailed`, `TransportServiceFailed`, `SendKeyValueFailed`
- No automatic retry mechanisms at API level
- Error callbacks are synchronous
- *Future improvement: Add configurable retry policies*

### Resource Constraints

**Memory Usage:**

- No built-in limits on property count or command history
- Property cache grows unbounded without manual cleanup
- *Future improvement: Add configurable memory limits and cleanup policies*

**Threading:**

- Single-threaded event processing within service
- User callbacks executed on service thread
- *Note: User code should avoid blocking operations in callbacks*

## Error Handling

### Service Errors

```cpp
service.subscribeOnError([](EErrorCode code, const std::string& message) {
    switch (code) {
        case EErrorCode::ConnectionFailed:
            LOG(error) << "Failed to connect to DDS commander: " << message;
            // Implement retry logic or fallback mode
            break;
            
        case EErrorCode::TransportServiceFailed:
            LOG(error) << "DDS services not available: " << message;
            // Check if DDS session is running
            break;
            
        case EErrorCode::SendKeyValueFailed:
            LOG(error) << "Property update failed: " << message;
            // Cache property for retry
            break;
    }
});
```

### Connection Resilience

- **Automatic Reconnection**: 2-minute timeout with exponential backoff
- **Service Discovery**: Automatically locates DDS commander
- **Graceful Degradation**: Continues operation during temporary disconnections

## Integration Examples

### With Existing Applications

```cpp
// Integrate DDS communication into existing application
class MyApplication {
    CIntercomService m_ddsService;
    CKeyValue m_ddsProps;
    
public:
    MyApplication() : m_ddsProps(m_ddsService) {
        // Report application status via DDS
        m_ddsProps.putValue("app_status", "initializing");
        
        // Monitor configuration changes
        m_ddsProps.subscribe([this](const std::string& key, const std::string& value, uint64_t sender) {
            if (key == "config_update") {
                reloadConfiguration(value);
            }
        });
    }
    
    void start() {
        m_ddsProps.putValue("app_status", "running");
        m_ddsService.start();
    }
    
    void reportProgress(float progress) {
        m_ddsProps.putValue("progress", std::to_string(progress));
    }
};
```

### With External Message Systems

```cpp
// Bridge DDS with external message queue
class MessageBridge {
    CIntercomService m_ddsService;
    CCustomCmd m_ddsCmd;
    ExternalMessageQueue m_externalMQ;
    
public:
    MessageBridge() : m_ddsCmd(m_ddsService) {
        bridgeMessages();
    }
    
private:
    void bridgeMessages() {
        // Forward DDS commands to external system
        m_ddsCmd.subscribe([this](const std::string& cmd, const std::string& condition, uint64_t sender) {
            if (cmd.find("external_") == 0) {
                m_externalMQ.publish(cmd.substr(9), condition);
            }
        });
        
        // Forward external messages to DDS
        m_externalMQ.subscribe([this](const std::string& topic, const std::string& data) {
            m_ddsCmd.send("external_" + topic, data);
        });
    }
};
```

## Building and Installation

### CMake Integration

```cmake
find_package(DDS REQUIRED)
target_link_libraries(your_application DDS::dds-intercom-lib)
```

### Dependencies

- DDS Protocol Library
- Boost libraries (system, signals2, thread)
- C++11 or later

### Example Build

```bash
# Configure DDS build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local /path/to/dds
make -j$(nproc)
make install

# Use in your project
find_package(DDS REQUIRED)
target_link_libraries(my_app DDS::dds-intercom-lib)
```

## Testing

The library includes comprehensive tests and examples:

```bash
# Run intercom library tests
cd build
make test

# Run tutorial examples
./dds-tutorials/dds-tutorial2/task-custom-cmd
./dds-tutorials/dds-tutorial2/ui-custom-cmd
```

## Related Documentation

- **[High-Level Protocol](../docs/protocol-high-level.md)** - Detailed protocol specification
- **[Low-Level Protocol](../docs/protocol-low-level.md)** - Binary protocol details
- **[Protocol Library](../dds-protocol-lib/README.md)** - Core implementation
- **[Tutorials](../docs/tutorials.md)** - Step-by-step examples

---

For complete examples and tutorials, see the [DDS tutorials](../docs/tutorials.md) and [examples directory](../dds-tutorials/).
