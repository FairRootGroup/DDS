//
//  DDSTaskGroup.h
//  DDS
//
//  Created by Andrey Lebedev on 2/12/14.
//
//

#ifndef DDS_DDSTaskGroup_h
#define DDS_DDSTaskGroup_h

// STD
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

using std::string;
using std::stringstream;
using std::endl;
using std::ostream;
using std::vector;
using std::for_each;

class DDSTaskGroup
{
public:
    /**
     * \brief Constructor.
     */
    DDSTaskGroup():
    m_name(""),
    m_taskCollections() {
        
    }
    
    /**
     * \brief Destructor.
     */
    virtual ~DDSTaskGroup() {}
    
    /**
     * \brief Returns number of task collections.
     * \return Number of task collections.
     */
    size_t getNofTaskCollections() const { m_taskCollections.size(); }
    
    const DDSTaskCollection& getTaskCollection(size_t i) const {
        asset(i < getNofTaskCollections()) // OR use exceptions?
        return m_taskCollections[i];
    }
    
    const vector<DDSTaskCollection>& getTaskCollections() const { return m_taskCollections; }
    
    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    string toString() const {
        stringstream ss;
        ss << "DDSTaskGroup: m_name=" << m_name << " m_depends=" << m_depends << endl
        << "   nof task collections=" << m_tasks.size() << " task collections:" << endl;
        for_each(m_taskCollections.begin(), m_taskCollections.end(), [&ss](const DDSTaskCollection& _v) mutable { ss << _v << " ";});
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
    string m_name; ///> Name of task group.
    vector<DDSTaskCollection> m_taskCollections; ///> Vector of task collections.
};

#endif
