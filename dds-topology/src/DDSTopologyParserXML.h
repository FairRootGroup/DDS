// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopologyParserXML__
#define __DDS__DDSTopologyParserXML__

// DDS
#include "DDSTaskGroup.h"
// STD
#include <string>
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
    void ParseMain(const boost::property_tree::ptree& _pt);

    void PrintPropertyTree(const std::string& _path, const boost::property_tree::ptree& _pt) const;

    DDSTaskGroupPtr_t m_main; ///> Main task group which we run
};

#endif /* defined(__DDS__DDSTopologyParserXML__) */
