// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoCreator__
#define __DDS__TopoCreator__

// STD
#include <memory>
// DDS
#include "TopoGroup.h"

namespace dds
{
    namespace topology_api
    {
        class CTopoCreatorCore;

        class CTopoCreator
        {
          public:
            /// \brief Constructor.
            CTopoCreator();

            /// \brief Constructs topology with the specified file and validates against provided schema file.
            /// \param[in] _filename Path to the topology file.
            /// \param[in] _schemaFilename Path to the XSD schema file.
            /// \throw runtime_error
            CTopoCreator(const std::string& _filename, const std::string& _schemaFilename = "");

            /// \brief Constructs topology from input stream and validates against provided schema file.
            /// \param[in] _stream Input stream.
            /// \param[in] _schemaFilename Path to the XSD schema file.
            /// \throw runtime_error
            CTopoCreator(std::istream& _stream, const std::string& _schemaFilename = "");

            /// \brief Destructor.
            virtual ~CTopoCreator();

            /// \brief Saves topology to the specified XML file.
            /// \param[in] _filename Path to the topology file.
            void save(const std::string& _filename);

            /// \brief Writes topology to stream.
            /// \param[in] _stream Output stream.
            void save(std::ostream& _stream);

            /// \brief Returns shared pointer to the main group of the topology.
            CTopoGroup::Ptr_t getMainGroup() const;

          private:
            std::shared_ptr<CTopoCreatorCore> m_topoCreator;
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoCreator__) */
