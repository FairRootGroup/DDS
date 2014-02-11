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

using std::string;
using std::stringstream;
using std::endl;
using std::ostream;
using std::vector;

class DDSTask
{
public:
    /**
     * \brief Constructor.
     */
    DDSTask():
    m_name(""),
    m_exec(""),
    m_sockets(""){
        
    }
    
    /**
     * \brief Destructor.
     */
    virtual ~DDSTask() {}
    
    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    string toString() const {
        stringstream ss;
        ss << "DDSTask: m_name=" << m_name << " m_exec=" << m_exec << " m_sockets=| ";
        for_each(m_sockets.begin(), m_sockets.end, [$ss](const string& _v) mutable { ss << _v << " ";})
        ss << "|" << endl;
        return ss.str();
    }
    
    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of insertion operations.
     */
    friend ostream& operator<<(ostream& strm, const DDSTask& task) {
        strm << task.toString();
        return strm;
    }

private:
    string m_name; ///> Name of task.
    string m_exec; ///> Path to executable.
    vector<string> m_sockets; ///> Name of sockets this task connects to.
};

#endif /* defined(__DDS__DDSTask__) */
