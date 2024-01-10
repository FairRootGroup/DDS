# Requirements

## Server/UI

DDS UI/Server/WN run on Linux and Mac OS X.

### General requirements

- Incoming connection on dds-commander port (configurable).
- A C++17 or a higher compiler.
- [cmake](http://www.cmake.org/) 3.19 or higher.
- [BOOST](http://www.boost.org/) 1.75 or higher.
- shell: [BASH (or a compatible one)](http://en.wikipedia.org/wiki/Bash_(Unix_shell))

### Additional requirements for the SSH plug-in

- A public key access (or password less, via ssh-agent, for example) to destination worker nodes.

## Agents

- Outgoing connection on dds-commander's port range (configurable).  
This is required by dds-agent to be able to connect to DDS commander server
- shell: [BASH (or a compatible one)](http://en.wikipedia.org/wiki/Bash_(Unix_shell))
