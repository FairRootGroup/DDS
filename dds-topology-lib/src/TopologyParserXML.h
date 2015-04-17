// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopologyParserXML__
#define __DDS__TopologyParserXML__

// DDS
#include "TaskGroup.h"
#include "TopoVars.h"
// STD
#include <string>
#include <map>
// BOOST
#include <boost/property_tree/ptree.hpp>

namespace dds
{
    class CTopologyParserXML
    {
      public:
        /// \brief Constructor.
        CTopologyParserXML();

        /// \brief Destructor.
        virtual ~CTopologyParserXML();

        /// \brief Read topology from specified XML file.
        /// \param[in] _fileName Name of file with topology.
        /// \param[out] _main Main task group or nullptr in case of error.
        /// \param[in] _xmlValidationDisabled If tru than XML will not be validated against XSD.
        void parse(const std::string& _fileName, TaskGroupPtr_t _main, bool _xmlValidationDisabled = false);

        /// \brief Validate provided XML file against XSD using xmllint.
        /// \throw runtime_error
        bool isValid(const std::string& _fileName, bool _xmlValidationDisabled = false);

      private:
        /// \brief Print recursively property tree to std::cout.
        void PrintPropertyTree(const std::string& _path, const boost::property_tree::ptree& _pt) const;
    };
}
#endif /* defined(__DDS__TopologyParserXML__) */
