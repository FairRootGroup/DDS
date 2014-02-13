//
//  DDSTask.h
//  DDS
//
//  Created by Andrey Lebedev on 2/12/14.
//
//

#ifndef __DDS__DDSTask__
#define __DDS__DDSTask__

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class DDSTask
{
public:
    /**
     * \brief Constructor.
     */
    DDSTask():
    m_name(""),
    m_exec(""),
    m_sockets(){
        
    }
    
    /**
     * \brief Destructor.
     */
    virtual ~DDSTask() {}
    
    void setName(const std::string& name) { m_name = name; }
    void setExec(const std::string& exec) { m_exec = exec; }
    void setSockets(const std::vector<std::string>& sockets) { m_sockets = sockets; }
    
    std::string getName() const { return m_name; }
    std::string getExec() const { return m_exec; }
    const std::vector<std::string>& getSockets() const { return m_sockets; }
    
    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const {
        std::stringstream ss;
        ss << "DDSTask: m_name=" << m_name << " m_exec=" << m_exec << " m_sockets=| ";
        std::for_each(m_sockets.begin(), m_sockets.end(), [&ss](const std::string& _v) mutable { ss << _v << " ";});
        ss << "|";
        return ss.str();
    }
    
    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTask& _task) {
        _strm << _task.toString();
        return _strm;
    }

private:
    std::string m_name; ///> Name of task.
    std::string m_exec; ///> Path to executable.
    std::vector<std::string> m_sockets; ///> Name of sockets this task connects to.
};

#endif /* defined(__DDS__DDSTask__) */
