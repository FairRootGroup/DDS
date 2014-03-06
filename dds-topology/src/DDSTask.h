// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTask__
#define __DDS__DDSTask__

// DDS
#include "DDSTopoElement.h"
#include "DDSPort.h"
// STD
#include <sstream>
#include <string>
#include <memory>

class DDSTask : public DDSTopoElement
{
public:
    /**
     * \brief Constructor.
     */
    DDSTask()
        : DDSTopoElement()
        , m_exec("")
        , m_ports()
    {
        setType(DDSTopoElementType::TASK);
    }

    /**
     * \brief Destructor.
     */
    virtual ~DDSTask()
    {
    }

    /// Modifiers
    void setExec(const std::string& _exec)
    {
        m_exec = _exec;
    }

    void setPorts(const DDSPortVector_t& _ports)
    {
        m_ports = _ports;
    }

    void addPort(const DDSPort& _port)
    {
        m_ports.push_back(_port);
    }

    /**
     * \brief Inherited from DDSTopoElement.
     */
    virtual size_t getNofTasks() const
    {
        return 1;
    }

    /// Accessors
    std::string getExec() const
    {
        return m_exec;
    }

    size_t getNofPorts() const
    {
        return m_ports.size();
    }

    const DDSPort& getPort(size_t _i) const
    {
        if (_i >= getNofPorts())
            throw std::out_of_range("Out of range exception");
        return m_ports[_i];
    }

    const DDSPortVector_t& getPorts() const
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
        ss << "DDSTask: m_name=" << getName() << " m_exec=" << m_exec << " m_ports:";
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
    std::string m_exec;      ///> Path to executable
    DDSPortVector_t m_ports; ///> Ports
};

typedef std::shared_ptr<DDSTask> DDSTaskPtr_t;
typedef std::vector<DDSTaskPtr_t> DDSTaskPtrVector_t;

#endif /* defined(__DDS__DDSTask__) */
