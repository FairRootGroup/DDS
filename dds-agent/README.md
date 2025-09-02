# DDS Agent

The DDS Agent is a core component of the DDS (Dynamic Distributed System) framework responsible for managing user tasks on worker nodes. It provides task execution, shared memory communication, and intercom services for distributed computing environments.

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Shared Memory Transport](#shared-memory-transport)
- [User Task Handling](#user-task-handling)
- [Process Lifecycle](#process-lifecycle)
- [Configuration](#configuration)
- [Error Handling](#error-handling)

## Overview

The DDS Agent acts as a worker node daemon that:

- Connects to the DDS Commander for task coordination
- Manages multiple task execution slots
- Provides shared memory-based inter-process communication
- Handles task lifecycle (assignment, activation, monitoring, termination)
- Forwards messages between user tasks and the DDS network

### Key Components

- **AgentConnectionManager**: Main service coordinator and network management
- **CommanderChannel**: TCP connection to DDS Commander for control messages
- **SMIntercomChannel**: Shared memory channel for user task communication
- **Slot Management**: Isolated execution environments for user tasks

## Architecture

```mermaid
graph TB
    subgraph "DDS Agent Process"
        ACM[AgentConnectionManager]
        CC[CommanderChannel]
        SMC[SMIntercomChannel]
        
        subgraph "Thread Pools"
            MT[Main Threads]
            IT[Intercom Threads]
        end
        
        subgraph "Slot Management"
            S1[Slot 1]
            S2[Slot 2]
            SN[Slot N]
        end
    end
    
    subgraph "External Components"
        CMD[DDS Commander]
        UT1[User Task 1]
        UT2[User Task 2]
        UTN[User Task N]
    end
    
    subgraph "Shared Memory"
        IQ[Input Queues]
        OQ[Output Queues]
    end
    
    CMD <==> CC
    CC --> ACM
    ACM --> SMC
    SMC <==> IQ
    SMC <==> OQ
    IQ <==> UT1
    IQ <==> UT2
    IQ <==> UTN
    OQ <==> UT1
    OQ <==> UT2
    OQ <==> UTN
    
    S1 -.-> UT1
    S2 -.-> UT2
    SN -.-> UTN
    
    ACM --> MT
    ACM --> IT
```

### Threading Model

The agent uses a multi-threaded architecture:

- **Main Threads** (4 threads): Handle network I/O and commander communication
- **Intercom Threads** (6 + NumLeaderFW): Process shared memory messages and user task communication
- **Signal Handler**: Manages graceful shutdown on SIGINT/SIGTERM/SIGQUIT

## Shared Memory Transport

The DDS Agent implements a sophisticated shared memory transport system for high-performance inter-process communication.

### Message Queue Architecture

```mermaid
graph LR
    subgraph "Agent Process"
        Agent[DDS Agent]
        SMC[SMIntercomChannel]
    end
    
    subgraph "Shared Memory Queues"
        subgraph "Input Queues"
            IQ1[Leader Input Queue 1]
            IQ2[Leader Input Queue 2]
            IQN[Leader Input Queue N]
        end
        
        subgraph "Output Queues"
            OQ1[Leader Output Queue Slot1]
            OQ2[Leader Output Queue Slot2]
            OQN[Leader Output Queue SlotN]
        end
    end
    
    subgraph "User Tasks"
        UT1[User Task Slot 1]
        UT2[User Task Slot 2]
        UTN[User Task Slot N]
    end
    
    Agent --> SMC
    SMC <==> IQ1
    SMC <==> IQ2
    SMC <==> IQN
    SMC <==> OQ1
    SMC <==> OQ2
    SMC <==> OQN
    
    UT1 <==> IQ1
    UT1 <==> OQ1
    UT2 <==> IQ2
    UT2 <==> OQ2
    UTN <==> IQN
    UTN <==> OQN
```

### Queue Management

The shared memory system uses **boost::interprocess::message_queue** with the following characteristics:

**Queue Types:**

- **Input Queues**: Multiple queues for receiving messages from user tasks
- **Output Queues**: Per-slot queues for sending messages to specific user tasks
- **Message Routing**: Agent forwards messages between network and shared memory

**Queue Naming Convention:**

```cpp
// Input queues (from user tasks to agent)
std::vector<std::string> inputNames = userDefaults.getSMLeaderInputNames();

// Output queues (from agent to specific user task)
std::string outputName = userDefaults.getSMLeaderOutputName(protocolHeaderID);
```

**Message Flow:**

1. User tasks write to input queues
2. Agent reads from all input queues
3. Agent processes and routes messages
4. Agent writes responses to specific output queues
5. User tasks read from their assigned output queue

### Memory Management

```mermaid
sequenceDiagram
    participant Agent
    participant SMChannel
    participant InputQueue
    participant OutputQueue
    participant UserTask
    
    Agent->>SMChannel: Initialize with queue names
    SMChannel->>InputQueue: Create/Open input queues
    SMChannel->>OutputQueue: Create/Open output queue
    
    UserTask->>InputQueue: Send message
    SMChannel->>InputQueue: Poll for messages
    InputQueue-->>SMChannel: Deliver message
    SMChannel->>Agent: Process message
    Agent->>SMChannel: Send response
    SMChannel->>OutputQueue: Write response
    UserTask->>OutputQueue: Read response
```

**Cleanup Process:**

- Agent removes message queues on shutdown
- Per-slot cleanup removes slot-specific output queues
- Graceful cleanup prevents orphaned shared memory objects

## User Task Handling

The DDS Agent manages user task execution through a sophisticated slot-based system.

### Task Lifecycle

```mermaid
stateDiagram-v2
    [*] --> SlotCreated: ADD_SLOT command
    SlotCreated --> TaskAssigned: ASSIGN_USER_TASK
    TaskAssigned --> TaskActivated: ACTIVATE_USER_TASK
    TaskActivated --> TaskRunning: Process spawned
    TaskRunning --> TaskCompleted: Normal exit
    TaskRunning --> TaskTerminated: STOP_USER_TASK
    TaskRunning --> TaskFailed: Process error
    TaskCompleted --> SlotAvailable: Clean slot
    TaskTerminated --> SlotAvailable: Clean slot
    TaskFailed --> SlotAvailable: Clean slot
    SlotAvailable --> TaskAssigned: New assignment
    SlotAvailable --> [*]: Agent shutdown
```

### Slot Information Structure

Each task slot maintains comprehensive metadata:

```cpp
struct SSlotInfo {
    slotId_t m_id;                    // Unique slot identifier
    std::string m_sUsrExe;            // User executable path
    std::string m_sUsrEnv;            // Environment setup script
    taskId_t m_taskID;                // DDS task ID
    uint32_t m_taskIndex;             // Task index in topology
    uint32_t m_collectionIndex;       // Collection index (if applicable)
    std::string m_taskPath;           // Task path in topology
    std::string m_groupName;          // Task group name
    std::string m_collectionName;     // Collection name
    std::string m_taskName;           // Task name
    pid_t m_pid;                      // Process ID of running task
    assets_t m_taskAssets;            // Task-specific file assets
};
```

### Task Assignment Process

```mermaid
sequenceDiagram
    participant Commander
    participant Agent
    participant Slot
    participant FileSystem
    
    Commander->>Agent: ASSIGN_USER_TASK
    Agent->>Agent: Validate topology hash
    Agent->>Slot: Create slot info
    Agent->>Agent: Replace path placeholders
    Note over Agent: %taskIndex% → actual index
    Note over Agent: %collectionIndex% → actual index  
    Note over Agent: %DDS_DEFAULT_TASK_PATH% → slot directory
    Agent->>FileSystem: Create slot directory
    Agent->>Agent: Enable intercom queue
    Agent->>Commander: Reply SUCCESS/ERROR
```

### Task Activation Process

```mermaid
sequenceDiagram
    participant Commander
    participant Agent
    participant TaskWrapper
    participant UserProcess
    participant Environment
    
    Commander->>Agent: ACTIVATE_USER_TASK
    Agent->>Environment: Set task environment variables
    Note over Environment: DDS_TASK_ID, DDS_TASK_INDEX,<br/>DDS_COLLECTION_INDEX, DDS_TASK_PATH,<br/>DDS_GROUP_NAME, DDS_COLLECTION_NAME,<br/>DDS_TASK_NAME, DDS_SLOT_ID
    Agent->>TaskWrapper: Generate task wrapper script
    Note over TaskWrapper: Source custom environment<br/>Execute user task
    Agent->>TaskWrapper: Apply execute permissions
    Agent->>UserProcess: spawn(bash wrapper_script)
    UserProcess->>Agent: Return PID
    Agent->>Agent: Register task monitoring
    Agent->>Commander: Reply with PID
```

### Task Environment Variables

The agent sets up a comprehensive environment for each user task:

| Variable               | Description                              | Example                          |
| ---------------------- | ---------------------------------------- | -------------------------------- |
| `DDS_TASK_ID`          | Unique task identifier                   | `12345`                          |
| `DDS_TASK_INDEX`       | Task index in topology                   | `0, 1, 2, ...`                   |
| `DDS_COLLECTION_INDEX` | Collection index (if part of collection) | `0, 1, 2, ...`                   |
| `DDS_TASK_PATH`        | Full path in topology                    | `/main/group1/collection1/task1` |
| `DDS_GROUP_NAME`       | Task group name                          | `group1`                         |
| `DDS_COLLECTION_NAME`  | Collection name                          | `collection1`                    |
| `DDS_TASK_NAME`        | Task name                                | `task1`                          |
| `DDS_SLOT_ID`          | Slot identifier                          | `1, 2, 3, ...`                   |
| `DDS_SESSION_ID`       | Session identifier                       | `uuid-string`                    |

### File Organization

```
$DDS_USER_HOME/
├── slots/
│   ├── 1/                           # Slot 1 directory
│   │   ├── dds_user_task_wrapper.sh # Generated wrapper script
│   │   ├── user_assets/             # Task-specific files
│   │   └── logs/                    # Task output logs
│   ├── 2/                           # Slot 2 directory
│   └── N/                           # Slot N directory
└── log/
    ├── task1_2024-01-01-12-00-00_12345_out.log
    └── task1_2024-01-01-12-00-00_12345_err.log
```

### Task Wrapper Script

The agent generates a wrapper script for each task execution:

```bash
#!/bin/bash
# Generated by DDS Agent

# Source custom environment (if provided)
source /path/to/custom/environment.sh

# Execute user task
/path/to/user/executable args...
```

## Process Lifecycle

### Task Monitoring

```mermaid
graph TB
    subgraph "Task Monitoring"
        TM[Task Monitor]
        RM[Resource Monitor]
        PM[Process Monitor]
    end
    
    subgraph "User Process"
        UP[User Task]
        CP[Child Processes]
        GP[Grandchild Processes]
    end
    
    subgraph "Monitoring Actions"
        DC[Disk Space Check]
        PC[Process Check]
        TC[Task Completion]
        TT[Task Termination]
    end
    
    TM --> RM
    TM --> PM
    RM --> DC
    PM --> PC
    PC --> TC
    PC --> TT
    
    UP --> CP
    CP --> GP
    
    PM -.-> UP
    PM -.-> CP
    PM -.-> GP
```

### Graceful Task Termination

The agent implements a sophisticated termination strategy:

1. **SIGTERM Phase** (5 seconds):
   - Send SIGTERM to main process
   - Enumerate all child and grandchild processes
   - Send SIGTERM to all descendants
   - Wait for graceful shutdown

2. **SIGKILL Phase**:
   - If processes still running after timeout
   - Send SIGKILL to all remaining processes
   - Force termination

```mermaid
sequenceDiagram
    participant Agent
    participant MainProcess
    participant ChildProcess
    participant GrandchildProcess
    
    Agent->>Agent: Enumerate process tree
    Agent->>MainProcess: SIGTERM
    Agent->>ChildProcess: SIGTERM
    Agent->>GrandchildProcess: SIGTERM
    
    Note over Agent: Wait 5 seconds
    
    alt Graceful shutdown
        MainProcess-->>Agent: Process exits
        ChildProcess-->>Agent: Process exits
        GrandchildProcess-->>Agent: Process exits
    else Force termination
        Agent->>MainProcess: SIGKILL
        Agent->>ChildProcess: SIGKILL
        Agent->>GrandchildProcess: SIGKILL
    end
    
    Agent->>Agent: Cleanup slot resources
```

### Resource Monitoring

The agent continuously monitors system resources:

- **Disk Space**: Prevents task execution when disk space is low
- **Memory Usage**: Tracks task memory consumption
- **Process Status**: Monitors task health and completion
- **Queue Status**: Manages shared memory queue overflow

## Configuration

### Agent Configuration Options

```cpp
struct SOptions_t {
    uint32_t m_slots;           // Number of task slots
    std::string m_groupName;    // Agent group name
    enum ECommand {
        cmd_start,              // Start agent
        cmd_clean              // Clean shared memory
    } m_Command;
};
```

### User Defaults Integration

The agent integrates with DDS User Defaults for:

- Session ID management
- Slot directory configuration
- Shared memory queue naming
- Resource limits and timeouts
- Access permissions for output files

### Server Connection

Agent connects to commander using configuration from server info file:

```ini
[server]
host=localhost
port=20000
```

## Error Handling

### Connection Management

```mermaid
graph LR
    subgraph "Connection States"
        C[Connected]
        D[Disconnected]
        R[Reconnecting]
        F[Failed]
    end
    
    subgraph "Error Responses"
        RT[Retry Timer]
        AC[Attempt Counter]
        SD[Shutdown]
    end
    
    C -->|Connection Lost| D
    D -->|Retry| R
    R -->|Success| C
    R -->|Failure| F
    F -->|Max Attempts| SD
    
    D --> RT
    R --> AC
    F --> AC
```

**Reconnection Strategy:**

- Maximum 5 connection attempts
- 5-second delay between attempts
- Exponential backoff for shared memory retries
- Automatic shutdown after maximum failures

### Task Error Handling

| Error Type              | Response                      | Recovery                 |
| ----------------------- | ----------------------------- | ------------------------ |
| Task Assignment Failure | Reply ERROR to commander      | Slot remains available   |
| Task Activation Failure | Reply ERROR to commander      | Clean slot state         |
| Process Spawn Failure   | Reply ERROR to commander      | Reset slot               |
| Task Crash              | Send TASK_DONE with exit code | Auto-cleanup slot        |
| Shared Memory Error     | Log error, retry              | Attempt queue recreation |
| Disk Space Full         | Reject new tasks              | Continue monitoring      |

### Cleanup Procedures

**Normal Shutdown:**

1. Stop accepting new connections
2. Terminate all running tasks gracefully
3. Remove shared memory queues
4. Clean slot directories
5. Remove agent ID file

**Emergency Cleanup:**

1. Force terminate all processes
2. Remove all message queues
3. Clean temporary files
4. Reset shared memory state

---

*This documentation covers the core functionality of the DDS Agent. For implementation details, refer to the source code in the `src/` directory.*
