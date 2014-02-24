//
//  DDSTask.h
//  DDS
//
//  Created by Andrey Lebedev on 2/12/14.
//
//

#ifndef __DDS__DDSTask__
#define __DDS__DDSTask__

// DDS
#include "DDSPort.h"
// STD
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

class DDSTask
{
public:
    /**
     * \brief Constructor.
     */
    DDSTask()
        : m_name("")
        , m_exec("")
        , m_ports()
    {
    }

    /**
     * \brief Destructor.
     */
    virtual ~DDSTask()
    {
    }

    /// Modifiers
    void setName(const std::string& _name)
    {
        m_name = _name;
    }

    void setExec(const std::string& _exec)
    {
        m_exec = _exec;
    }

    void setPorts(const std::vector<DDSPortPtr_t>& _ports)
    {
        m_ports = _ports;
    }

    void addPort(DDSPortPtr_t _port)
    {
        m_ports.push_back(_port);
    }

    /// Accessors
    std::string getName() const
    {
        return m_name;
    }

    std::string getExec() const
    {
        return m_exec;
    }

    size_t getNofPorts() const
    {
        return m_ports.size();
    }

    DDSPortPtr_t getPort(size_t _i) const
    {
        if (_i >= getNofPorts())
            throw std::out_of_range("Out of range exception");
        return m_ports[_i];
    }

    const std::vector<DDSPortPtr_t>& getPorts() const
    {
        return m_ports;
    }

    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const
    {
        std::stringstream ss;
        ss << "DDSTask: m_name=" << m_name << " m_exec=" << m_exec << " m_ports:";
        for (const auto& port : m_ports)
        {
            ss << port << std::endl;
        }
        return ss.str();
    }

    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of
     * insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTask& _task)
    {
        _strm << _task.toString();
        return _strm;
    }

private:
    std::string m_name;         ///> Name of task
    std::string m_exec;         ///> Path to executable
    DDSPortPtrVector_t m_ports; ///> Ports
};

typedef std::shared_ptr<DDSTask> DDSTaskPtr_t;
typedef std::vector<DDSTaskPtr_t> DDSTaskPtrVector_t;

#endif /* defined(__DDS__DDSTask__) */
