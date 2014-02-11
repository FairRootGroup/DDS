//
//  DDSTaskCollection.h
//  DDS
//
//  Created by Andrey Lebedev on 2/12/14.
//
//

#ifndef __DDS__DDSTaskCollection__
#define __DDS__DDSTaskCollection__
// DDS
#include "DDSTask.h"
// STD
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using std::string;
using std::stringstream;
using std::endl;
using std::ostream;
using std::vector;
using std::for_each;

class DDSTaskCollection
{
public:
    /**
     * \brief Constructor.
     */
    DDSTaskCollection():
    m_name(""),
    m_tasks() {
        
    }
    
    /**
     * \brief Destructor.
     */
    virtual ~DDSTaskCollection() {}
    
    size_t getNofTasks() const { m_tasks.size(); }
    
    const DDSTask& getTask(size_t i) const {
        asset(i < getNofTasks()) // OR use exceptions?
        return m_tasks[i];
    }
    
    const vector<DDSTask>& getTasks() const { return m_tasks; }
    
    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    string toString() const {
        stringstream ss;
        ss << "DDSTaskCollection: m_name=" << m_name << " m_depends=" << m_depends << endl
            << "   nof tasks=" << m_tasks.size() << " tasks:" << endl;
        for_each(m_tasks.begin(), m_tasks.end(), [&ss](const DDSTask& _v) mutable { ss << _v << " ";});
        return ss.str();
    }
    
    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of insertion operations.
     */
    friend ostream& operator<<(ostream& strm, const DDSTaskCollection& taskCollection) {
        strm << taskCollection.toString();
        return strm;
    }

private:
    string m_name; ///> Name of task collection.
    vector<DDSTask> m_tasks; ///> Vector of tasks in collection.
};

#endif /* defined(__DDS__DDSTopology__) */
