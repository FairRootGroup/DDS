# DDS Protocol Library

This library implements the core communication protocols for the Dynamic Deployment System (DDS). It provides both low-level binary protocol handling and high-level abstractions for distributed task communication in HPC environments.

## Overview

The DDS protocol operates at two distinct layers:

1. **[Low-Level Protocol](../docs/protocol-low-level.md)** - Efficient binary protocol with network and shared memory transport
2. **[High-Level Protocol](../docs/protocol-high-level.md)** - Custom commands, key-value store, and application APIs

## Key Features

- **Dual Transport**: TCP networking and shared memory for optimal performance
- **Binary Protocol**: Efficient serialization with CRC validation  
- **Async I/O**: Boost::asio-based implementation for high concurrency
- **Zero-Copy**: Shared memory transport eliminates data copying for local communication
- **Type Safety**: Template-based command serialization with compile-time validation
- **Message Accumulation**: Batches small messages to reduce system call overhead

## Protocol Components

### Core Protocol Classes

- `CProtocolMessage` - Binary message framing and encoding/decoding
- `SMessageHeader` - Fixed 16-byte message header with CRC validation
- `SBasicCmd<T>` - Template base for command serialization
- `CBaseChannelImpl<T>` - Network channel implementation
- `CBaseSMChannelImpl<T>` - Shared memory channel implementation

### Command Types

The protocol defines 40+ command types for:

- **Connection Management**: Handshake, shutdown, heartbeat
- **Task Lifecycle**: Assignment, activation, completion  
- **Communication**: Custom commands, property updates, messages
- **Data Transfer**: Binary attachments with chunking support
- **Information Exchange**: Agent status, host information, logs

### Transport Layer

**Network Transport (TCP):**
- Boost::asio async I/O with automatic reconnection
- Connection pooling and message accumulation
- Supports thousands of concurrent connections

**Shared Memory Transport:**
- Boost::interprocess message queues
- Zero-copy for local process communication
- Automatic cleanup on process termination

## Protocol Limits and Constraints

### Message and Data Limits

**Serialization Constraints:**
- All vectors (except `uint8_t`) have max size of 2^16 (65,536) elements
- `std::vector<uint8_t>` has max size of 2^32 (4,294,967,296) bytes
- `std::string` has max size of 2^16 (65,536) characters
- Protocol header: Fixed 16-byte overhead per message

### Queue and Buffer Configuration

**Network Channel Limits:**
- Max accumulative write queue size: **10,000 messages** (hardcoded in `BaseChannelImpl.h`)
- Deadline timer for message batching: **10ms** (hardcoded)
- *Future improvement: Make queue sizes and timing user-configurable*

**Shared Memory Limits:**
- Max message size: **512KB** (hardcoded in `BaseSMChannelImpl.h`)
- Note: Previously reduced from 65KB to 2KB due to performance issues
- Max messages per queue: Configurable at message queue creation
- *Future improvement: Make message size limits configurable*

### Protocol Version Constraints

**Version Management:**
- Current protocol version: **5** (defined in `g_protocolCommandsVersion`)
- No backward compatibility implemented
- Breaking changes require version increment and connection rejection

### Transport Limitations

**Connection Management:**
- Reconnection timeout: **2 minutes maximum** (hardcoded)
- No manual transport selection (automatic TCP/shared memory)
- *Future improvement: Add configurable timeout and manual transport override*

**Binary Transfer Support:**
- Transport testing supports up to **10MB** transfers (`ConnectionManager.cpp`)
- Chunking implemented for large binary attachments
- Test sizes: 1KB, 10KB, 100KB, 1MB, 10MB

## Usage Examples

### Basic Command Implementation

```cpp
#include "BasicCmd.h"

struct SMyCommand : public SBasicCmd<SMyCommand> {
    uint64_t m_userId;
    std::string m_action;
    std::vector<uint32_t> m_parameters;
    
    size_t size() const {
        return dsize(m_userId) + dsize(m_action) + dsize(m_parameters);
    }
    
    void _convertToData(dds::misc::BYTEVector_t* _data) const {
        SAttachmentDataProvider(*_data)
            .put(m_userId)
            .put(m_action)
            .put(m_parameters);
    }
    
    void _convertFromData(const dds::misc::BYTEVector_t& _data) {
        SAttachmentDataProvider(_data)
            .get(m_userId)
            .get(m_action)
            .get(m_parameters);
    }
};
```

### Network Channel Usage

```cpp
#include "ClientChannelImpl.h"

class MyChannel : public CClientChannelImpl<MyChannel> {
public:
    MyChannel(boost::asio::io_context& service, uint64_t protocolId)
        : CClientChannelImpl<MyChannel>(service, EChannelType::UI, protocolId) {
        
        // Register message handlers
        registerHandler<cmdSIMPLE_MSG>(
            [this](const SSenderInfo& sender, SCommandAttachmentImpl<cmdSIMPLE_MSG>::ptr_t attachment) {
                handleMessage(attachment->m_sMsg);
            });
    }
    
    void sendCommand(const std::string& command) {
        SSimpleMsgCmd cmd(command, dds::misc::info);
        pushMsg<cmdSIMPLE_MSG>(cmd);
    }
};
```

### Shared Memory Channel

```cpp
#include "BaseSMChannelImpl.h"

// Create shared memory channel
auto channel = CSMChannel::makeNew(
    ioContext,
    "input_queue_name",
    "output_queue_name", 
    protocolHeaderId
);

// Send command via shared memory
SCustomCmdCmd cmd;
cmd.m_sCmd = "process_data";
cmd.m_timestamp = getCurrentTimestamp();
channel->pushMsg<cmdCUSTOM_CMD>(cmd, outputId);
```

## Thread Safety

The protocol library is designed for multi-threaded environments:

- **Async Operations**: All I/O operations use boost::asio thread pools
- **Message Queues**: Protected by mutexes with minimal lock contention
- **Shared Ownership**: Uses std::shared_ptr for lifetime management
- **Signal Safety**: Boost::signals2 for thread-safe event handling

## Error Handling

### Protocol-Level Errors

- **CRC Validation**: Detects corrupted messages automatically
- **Length Validation**: Prevents buffer overflow attacks
- **Connection Errors**: Automatic retry with exponential backoff
- **Transport Failures**: Graceful fallback between transports

### Application-Level Errors

```cpp
// Error handling in channel implementations
void handleError(const boost::system::error_code& error) {
    if (error == boost::asio::error::connection_reset) {
        // Attempt reconnection
        scheduleReconnect();
    } else if (error == boost::asio::error::message_size) {
        // Protocol violation - log and close
        LOG(error) << "Invalid message size received";
        close();
    }
}
```

## Testing

The protocol library includes comprehensive tests:

- **Unit Tests**: Protocol message encoding/decoding
- **Integration Tests**: Channel communication patterns
- **Performance Tests**: Throughput and latency benchmarks
- **Stress Tests**: High-concurrency scenarios with resource limits

Run tests with:

```bash
cd build
make test
# or
ctest -V
```

## Dependencies

- **Boost Libraries**: asio, interprocess, system, signals2, uuid
- **C++ Standard**: C++11 or later
- **Platform**: Linux, macOS, Windows (with minor platform-specific code)

## Integration

To use the protocol library in your application:

```cmake
find_package(DDS REQUIRED)
target_link_libraries(your_target DDS::dds-protocol-lib)
```

```cpp
#include "Intercom.h"  // High-level API
#include "ProtocolMessage.h"  // Low-level protocol
```

## Related Documentation

- **[Low-Level Protocol Specification](../docs/protocol-low-level.md)** - Binary protocol details
- **[High-Level Protocol Specification](../docs/protocol-high-level.md)** - Application APIs  
- **[Intercom Library](../dds-intercom-lib/README.md)** - User-facing communication API
- **[Tools Library](../dds-tools-lib/README.md)** - Integration utilities

## Contributing

When modifying the protocol:

1. **Maintain Compatibility**: Changes must be backward compatible
2. **Update Version**: Increment `g_protocolCommandsVersion` for breaking changes
3. **Add Tests**: Include unit tests for new commands or features
4. **Benchmark**: Verify performance impact with provided benchmarks
5. **Document**: Update protocol specification documents

## Performance Tuning

### Network Optimization

```cpp
// Tune accumulative message sending
static const size_t maxAccumulativeWriteQueueSize = 10000;  // Adjust based on workload
static const std::chrono::milliseconds deadlineTimerMs{10}; // Batch window
```

### Shared Memory Optimization

```cpp
// Configure message queue parameters
static const size_t maxNofMessages = 256;     // Queue depth
static const size_t maxMessageSize = 512000;  // Max message size
```

### Connection Management

```cpp
// Tune reconnection parameters
static const std::chrono::seconds reconnectTimeoutMs{120}; // Max retry time
static const size_t maxConnectionRetries = 10;            // Retry attempts
```

---

For detailed protocol specifications and usage examples, see the full documentation in the [docs/](../docs/) directory.
