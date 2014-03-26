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
#include <string>
#include <memory>

class DDSTask : public DDSTopoElement
{
  public:
    /// \brief Constructor.
    DDSTask();

    /// \brief Destructor.
    virtual ~DDSTask();

    /// Accessors
    std::string getExec() const;
    size_t getNofPorts() const;
    DDSPortPtr_t getPort(size_t _i) const;
    const DDSPortPtrVector_t& getPorts() const;

    /// Modifiers
    void setExec(const std::string& _exec);
    void setPorts(const DDSPortPtrVector_t& _ports);
    void addPort(DDSPortPtr_t& _port);

    /// \brief Inherited from DDSTopoElement.
    virtual size_t getNofTasks() const;
    
    /// \brief Inherited from DDSTopoElement.
    virtual size_t getTotalNofTasks() const;
    
    /// \brief Inherited from DDSTopoElement.
    virtual size_t getMinRequiredNofTasks() const;

    /// \brief Inherited from DDSTopoElement.
    virtual void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

    /// \brief Returns string representation of an object.
    /// \return String representation of an object.
    virtual std::string toString() const;

    /// \brief Operator << for convenient output to ostream.
    /// \return Insertion stream in order to be able to call a succession of
    /// insertion operations.
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTask& _task);

  private:
    std::string m_exec;         ///> Path to executable
    DDSPortPtrVector_t m_ports; ///> Ports
};

typedef std::shared_ptr<DDSTask> DDSTaskPtr_t;
typedef std::vector<DDSTaskPtr_t> DDSTaskPtrVector_t;

#endif /* defined(__DDS__DDSTask__) */
