// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopologyParserXML__
#define __DDS__TopologyParserXML__

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
            /// \brief Parse  topology from specified XML file.
            /// \param[out] _pt Output property tree.
            /// \param[in] _filepath Path to the topology file.
            /// \param[in] _schemaFilepath Path to the XSD schema file.
            /// \throw std::runtime_error.
            static void parse(boost::property_tree::ptree& _pt,
                              const std::string& _filepath,
                              const std::string& _schemaFilepath,
                              std::string* _topologyName = nullptr);

            /// \brief Parse  topology from specified XML input stream.
            /// \param[out] _pt Output property tree.
            /// \param[in] _stream Input stream.
            /// \param[in] _schemaFilepath Path to the XSD schema file.
            /// \throw std::runtime_error.
            static void parse(boost::property_tree::ptree& _pt,
                              std::istream& _stream,
                              const std::string& _schemaFilepath,
                              std::string* _topologyName = nullptr);

            /// \brief Validate provided XML file against XSD using xmllint.
            /// \param[in] _filepath Path to the topology file.
            /// \param[in] _schemaFilepath Path to the XSD schema file.
            /// \return true if file is valid or schema filepath is empty, otherwise return false.
            /// \throw std::runtime_error
            static bool isValid(const std::string& _filepath,
                                const std::string& _schemaFilepath,
                                std::string* _output = nullptr);

            /// \brief Validate provided XML input stream against XSD using xmllint.
            /// \param[in] _stream Input stream.
            /// \param[in] _schemaFilepath Path to the XSD schema file.
            /// \return true if file is valid or schema filepath is empty, otherwise return false.
            /// \throw std::runtime_error
            static bool isValid(std::istream& _stream,
                                const std::string& _schemaFilepath,
                                std::string* _output = nullptr);

          private:
            /// \brief Print recursively property tree to std::cout.
            static void PrintPropertyTree(const std::string& _path, const boost::property_tree::ptree& _pt);
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopologyParserXML__) */
