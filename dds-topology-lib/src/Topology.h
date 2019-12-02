// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_TOPOLOGY_H
#define DDS_TOPOLOGY_H

// STD
#include <memory>
// DDS
#include "TopoCollection.h"
#include "TopoGroup.h"
#include "TopoTask.h"

namespace dds
{
    namespace topology_api
    {
        class CTopoCore;

        class CTopology
        {
          public:
            /// \brief Default constructor
            CTopology();

            /// \brief Destructor
            ~CTopology();

            /// \brief Constructs and initializes topology with the specified file without validation.
            /// \param[in] _fileName Path to the topology file
            /// \throw runtime_error
            CTopology(const std::string& _fileName);

            /// \brief Constructs and initializes topology with the specified file and validates against provided schema
            /// file. \param[in] _fileName Path to the topology file. \param[in] _schemaFileName Path to the XSD schema
            /// file. \throw runtime_error
            CTopology(const std::string& _fileName, const std::string& _schemaFileName);

            /// \brief Returns topology name
            /// \throw runtime_error
            std::string getName() const;

            /// \brief Returns topology hash
            /// \throw runtime_error
            uint32_t getHash() const;

            /// \brief Returns shared pointer to the main group of the topology
            CTopoGroup::Ptr_t getMainGroup() const;

            /// \brief Returns runtime task by ID.
            /// \param[in] _id Runtime task ID.
            const STopoRuntimeTask& getRuntimeTaskById(Id_t _id) const;

            /// \brief Returns runtime collection by ID.
            /// \param[in] _id Runtime collection ID.
            const STopoRuntimeCollection& getRuntimeCollectionById(Id_t _id) const;

            /// \brief Returns runtime task by path.
            /// \param[in] _idPath Runtime task path in the topology.
            const STopoRuntimeTask& getRuntimeTaskByIdPath(const std::string& _idPath) const;

            /// \brief Returns runtime collection by path.
            /// \param[in] _idPath Runtime collection path in the topology.
            const STopoRuntimeCollection& getRuntimeCollectionByIdPath(const std::string& _idPath) const;

            /// \brief Returns runtime task filter iterator.
            /// \param[in] _condition If provided than iterate over tasks passed the condition.
            STopoRuntimeTask::FilterIteratorPair_t getRuntimeTaskIterator(
                STopoRuntimeTask::Condition_t _condition = nullptr) const;

            /// \brief Returns runtime collection filter iterator.
            /// \param[in] _condition If provided than iterate over collections passed the condition.
            STopoRuntimeCollection::FilterIteratorPair_t getRuntimeCollectionIterator(
                STopoRuntimeCollection::Condition_t _condition = nullptr) const;

            /// \brief Returns runtime task filter iterator matching the task path in the topology.
            /// \param[in] _pathPattern Regex for task path in the topology.
            STopoRuntimeTask::FilterIteratorPair_t getRuntimeTaskIteratorMatchingPath(
                const std::string& _pathPattern) const;

            /// \brief Returns runtime collection filter iterator matching the collection path in the topology.
            /// \param[in] _pathPattern Regex for collection path in the topology.
            STopoRuntimeCollection::FilterIteratorPair_t getRuntimeCollectionIteratorMatchingPath(
                const std::string& _pathPattern) const;

            /// \brief Returns required number of agents and slots for the topology as std::pair.
            /// \warning Limitations: this function might return a wrong results for topologies containing requirements
            /// and/or different number of tasks per collection. You might need to play with _defaultNumSlots to get a
            /// right result for those cases. \param[in] _defaultNumSlots Default number of slots per agent. \return
            /// std::pair of a number of agents (first) and slots (second).
            std::pair<size_t, size_t> getRequiredNofAgents(size_t _defaultNumSlots) const;

            /// \warning This finction is depricated, use std::pair<size_t, size_t> getRequiredNofAgents(size_t
            /// _defaultNumSlots) const instead \brief Returns required number of agents and slots for the topology as
            /// std::pair.
            size_t getRequiredNofAgents() const;

            /// \brief Returns total number of tasks in the topology.
            size_t getTotalNofTasks() const;

          private:
            std::shared_ptr<CTopoCore> m_topo;
        };
    } // namespace topology_api
} // namespace dds

#endif /* DDS_TOPOLOGY_H */
