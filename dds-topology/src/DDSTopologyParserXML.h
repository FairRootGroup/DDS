// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopologyParserXML__
#define __DDS__DDSTopologyParserXML__

// DDS
#include "DDSTaskContainer.h"
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

class DDSTopologyParserXML
{
public:
    /**
     * \brief Constructor.
     */
    DDSTopologyParserXML();

    /**
     * \brief Destructor.
     */
    virtual ~DDSTopologyParserXML();

    /**
     * \brief Read topology from specified XML file.
     * \return Main task group or nullptr in case of error.
     */
    DDSTaskGroupPtr_t parse(const std::string& _fileName);

    /**
     * \brief Validate provided XML file against XSD using xmllint.
     * \throw runtime_error
     */
    bool isValid(const std::string& _fileName);

private:
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

    DDSStringToPortPtrMap_t m_tempPorts;                 ///> Temporary storage for all ports
    DDSStringToTaskPtrMap_t m_tempTasks;                 ///> Temporary storage for all tasks
    DDSStringToTaskCollectionPtrMap_t m_tempCollections; ///> Temporary storage for all task collections
    DDSStringToTaskGroupPtrMap_t m_tempGroups;           ///> Temporary storage for all task groups

    DDSTaskGroupPtr_t m_main; ///> Main task group which we run
};

#endif /* defined(__DDS__DDSTopologyParserXML__) */
