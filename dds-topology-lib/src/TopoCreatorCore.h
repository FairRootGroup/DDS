// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoCreatorCore__
#define __DDS__TopoCreatorCore__

// STD
#include <map>
// DDS
#include "TopoCollection.h"
#include "TopoGroup.h"
#include "TopoTask.h"

namespace dds
{
    namespace topology_api
    {
        class CTopoCreatorCore
        {
          public:
            /// \brief Constructor.
            CTopoCreatorCore();

            /// \brief Destructor.
            virtual ~CTopoCreatorCore();

            /// \brief Initializes default topology.
            /// \throw runtime_error
            void init();

            /// \brief Initializes topology with the specified file without validation.
            /// \param[in] _filename Path to the topology file
            /// \throw runtime_error
            void init(const std::string& _filename);

            /// \brief Initializes topology with the specified file and validates against provided schema file.
            /// \param[in] _filename Path to the topology file.
            /// \param[in] _schemaFilename Path to the XSD schema file.
            /// \throw runtime_error
            void init(const std::string& _filename, const std::string& _schemaFilename);

            /// \brief Saves topology to the specified XML file.
            /// \param[in] _filename Path to the topology file.
            void save(const std::string& _filename);

            /// \brief Writes topology to stream.
            /// \param[in] _filename Path to the topology file.
            void save(std::ostream& _stream);

            /// \brief Returns shared pointer to the main group of the topology.
            CTopoGroup::Ptr_t getMainGroup() const;

          private:
            typedef std::map<CTopoBase::EType, std::map<std::string, CTopoBase::Ptr_t>> declElementsMap_t;

            void addDeclElements(CTopoElement::Ptr_t _element, declElementsMap_t& _declElements);
            void addDeclElements(CTopoTask::Ptr_t _task, declElementsMap_t& _declElements);
            void addDeclElements(CTopoCollection::Ptr_t _collection, declElementsMap_t& _declElements);
            void addDeclElements(CTopoGroup::Ptr_t _group, declElementsMap_t& _declElements);

            void save(boost::property_tree::ptree& _pt);

            CTopoGroup::Ptr_t m_main; ///< Main group of the topology
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoCreatorCore__) */
