# Requirements

## Server/UI

DDS UI/Server/WN run on Linux and Mac OS X.

### General requirements

- Incoming connection on dds-commander port (configurable via `server.commander_port_range_min` and `server.commander_port_range_max` settings).
- A C++17 or a higher compiler.
- [cmake](http://www.cmake.org/) 3.19 or higher.
- [BOOST](http://www.boost.org/) 1.75 or higher.
- shell: [BASH (or a compatible one)](http://en.wikipedia.org/wiki/Bash_(Unix_shell))

For details about port configuration and other settings, see the [User Defaults Configuration Reference](user-defaults-configuration.md).

### Additional requirements for the SSH plug-in

- A public key access (or password less, via ssh-agent, for example) to destination worker nodes.

## Agents

- Outgoing connection on dds-commander's port range (configurable via `server.commander_port_range_min` and `server.commander_port_range_max` settings).  
This is required by dds-agent to be able to connect to DDS commander server
- shell: [BASH (or a compatible one)](http://en.wikipedia.org/wiki/Bash_(Unix_shell))
