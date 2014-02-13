//
//  DDSTopology.h
//  DDS
//
//  Created by Andrey Lebedev on 2/12/14.
//
//

#ifndef __DDS__DDSTopology__
#define __DDS__DDSTopology__

// DDS
#include "DDSTask.h"
#include "DDSTaskCollection.h"
#include "DDSTaskGroup.h"
// STD
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
// BOOST
#include <boost/property_tree/ptree.hpp>

class DDSTopology
{
public:
    
    /**
     * \brief Constructor.
     */
    DDSTopology();
    
    /**
     * \brief Destructor.
     */
    virtual ~DDSTopology();
    
    /**
     * \brief Read topology from specified XML file.
     */
    void init(const std::string& _fileName);
    
    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const {
        std::stringstream ss;
        ss << "DDSTopology:" << std::endl;
        for_each(m_tasks.begin(), m_tasks.end(), [&ss] (const DDSTask& _task) { ss << _task << std::endl; });
        for_each(m_collections.begin(), m_collections.end(), [&ss] (const DDSTaskCollection& _taskCollection) { ss << _taskCollection << std::endl; });
        for_each(m_groups.begin(), m_groups.end(), [&ss] (const DDSTaskGroup& _taskGroup) { ss << _taskGroup << std::endl; });
        return ss.str();
    }
    
    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTopology& _topology) {
        _strm << _topology.toString();
        return _strm;
    }

private:
    void ParsePropertyTree(
        const boost::property_tree::ptree& _pt);
    
    void ParsePropertyTreeTask(
        const boost::property_tree::ptree& _pt);
    
    void ParsePropertyTreeSocket(
        const boost::property_tree::ptree& _pt);
    
    void ParsePropertyTreeCollection(
        const boost::property_tree::ptree& _pt);
    
    void ParsePropertyTreeGroup(
        const boost::property_tree::ptree& _pt);
    
    void PrintPropertyTree(
         const std::string& _path,
         const boost::property_tree::ptree& _pt) const;
    
    std::vector<DDSTask> m_tasks;
    std::vector<DDSTaskCollection> m_collections;
    std::vector<DDSTaskGroup> m_groups;
    
};

#endif /* defined(__DDS__DDSTopology__) */
