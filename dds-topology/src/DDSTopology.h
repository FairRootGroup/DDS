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
    std::string toString() const
    {
        std::stringstream ss;
        //        ss << "DDSTopology:" << std::endl;
        //        for (const auto& socket : m_sockets)
        //        {
        //            ss << socket << std::endl;
        //        }
        //        for (const auto& task : m_tasks)
        //        {
        //            ss << task << std::endl;
        //        }
        //        for (const auto& collection : m_collections)
        //        {
        //            ss << collection << std::endl;
        //        }
        //        for (const auto& group : m_groups)
        //        {
        //            ss << group << std::endl;
        //        }
        return ss.str();
    }

    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of
     * insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTopology& _topology)
    {
        _strm << _topology.toString();
        return _strm;
    }

private:
    void ParsePropertyTree(const boost::property_tree::ptree& _pt);

    void ParseTask(const boost::property_tree::ptree& _pt);

    void ParsePort(const boost::property_tree::ptree& _pt);

    void ParseTaskCollection(const boost::property_tree::ptree& _pt);

    void ParseTaskGroup(const boost::property_tree::ptree& _pt);

    void ParseMain(const boost::property_tree::ptree& _pt);

    void PrintPropertyTree(const std::string& _path, const boost::property_tree::ptree& _pt) const;

    typedef std::map<std::string, DDSTaskPtr_t> DDSStringToTaskPtrMap_t;
    typedef std::map<std::string, DDSTaskCollectionPtr_t> DDSStringToTaskCollectionPtrMap_t;
    typedef std::map<std::string, DDSTaskGroupPtr_t> DDSStringToTaskGroupPtrMap_t;
    typedef std::map<std::string, DDSPortPtr_t> DDSStringToPortPtrMap_t;

    DDSStringToTaskPtrMap_t m_tasks;                 ///> Temporary storage for all tasks
    DDSStringToTaskCollectionPtrMap_t m_collections; ///> Temporary storage for all task collections
    DDSStringToTaskGroupPtrMap_t m_groups;           ///> Temporary storage for all task groups
    DDSStringToPortPtrMap_t m_ports;                 ///> Temporary storage for all ports

    DDSTaskGroupPtr_t m_mainGroup; ///> Main task group which we run
    size_t m_nofMainGroups;        ///> Number of requested main groups
};

#endif /* defined(__DDS__DDSTopology__) */
