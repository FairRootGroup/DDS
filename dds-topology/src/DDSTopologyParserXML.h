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
    /// \brief Constructor.
    DDSTopologyParserXML();

    /// \brief Destructor.
    virtual ~DDSTopologyParserXML();

    /// \brief Read topology from specified XML file.
    /// \param[in] _fileName Name of file with topology.
    /// \param[in] _main Main task group or nullptr in case of error.
    void parse(const std::string& _fileName, DDSTaskGroupPtr_t _main);

    /// \brief Validate provided XML file against XSD using xmllint.
    /// \throw runtime_error
    bool isValid(const std::string& _fileName);

  private:
    /// \brief Print recursively property tree to std::cout.
    void PrintPropertyTree(const std::string& _path, const boost::property_tree::ptree& _pt) const;
};

#endif /* defined(__DDS__DDSTopologyParserXML__) */
