// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopologyParserXML__
#define __DDS__TopologyParserXML__

// DDS
#include "TopoGroup.h"
// STD
#include <string>
// BOOST
#include <boost/property_tree/ptree.hpp>

namespace dds
{
    namespace topology_api
    {
        class CTopoParserXML
        {
          public:
            /// \brief Constructor.
            CTopoParserXML();

            /// \brief Destructor.
            virtual ~CTopoParserXML();

            /// \brief Read topology from specified XML file.
            /// \param[in] _fileName Name of file with topology.
            /// \param[out] _main Main task group or nullptr in case of error.
            /// \param[in] _xmlValidationDisabled If true than XML will not be validated against XSD.
            void parse(const std::string& _fileName, const std::string& _schemaFileName, CTopoGroup::Ptr_t _main);

            /// \brief Read topology from specified XML file.
            /// \param[in] _fileName Name of file with topology.
            /// \param[out] _main Main task group or nullptr in case of error.
            /// \param[in] _xmlValidationDisabled If true than XML will not be validated against XSD.
            /// \param[out] _name Topology name.
            void parse(const std::string& _fileName, const std::string& _schemaFileName, CTopoGroup::Ptr_t _main, std::string& _name);

            /// \brief Validate provided XML file against XSD using xmllint.
            /// \throw runtime_error
            bool isValid(const std::string& _fileName,
                         const std::string& _schemaFileName,
                         std::string* _output = nullptr);

          private:
            /// \brief Print recursively property tree to std::cout.
            void PrintPropertyTree(const std::string& _path, const boost::property_tree::ptree& _pt) const;
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopologyParserXML__) */
