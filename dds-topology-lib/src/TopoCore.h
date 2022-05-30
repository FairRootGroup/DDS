// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__Topology__
#define __DDS__Topology__

// DDS Topo
#include "TopoCollection.h"
#include "TopoDef.h"
#include "TopoElement.h"
#include "TopoGroup.h"
#include "TopoTask.h"
// STD
#include <map>
#include <mutex>
#include <ostream>
#include <set>
#include <string>

namespace dds
{
    namespace topology_api
    {
        class CTopoCore
        {
          public:
            /// Note that ID is of type uint_64.
            /// ID is calculated using CRC64 algorithm.
            using IdSet_t = std::set<Id_t>;

            /// Task/Collection ID path  to Task/Collection ID map
            using IdPathToIdMap_t = std::map<std::string, Id_t>;

            /// std::shared_ptr
            using Ptr_t = std::shared_ptr<CTopoCore>;

          public:
            /// \brief Constructor.
            CTopoCore();
            CTopoCore(CTopoCore const& _topo);
            CTopoCore& operator=(CTopoCore const& _topo);

            /// \brief Destructor.
            virtual ~CTopoCore();

            /// \brief Initializes topology from specified file.
            /// \throw runtime_error
            void init(const std::string& _fileName);

            /// \brief Initializes topology from specified file and validates with provided schema file.
            /// \throw runtime_error
            void init(const std::string& _fileName, const std::string& _schemaFileName);

            /// \brief Initializes topology from input stream
            /// \throw runtime_error
            void init(std::istream& _stream);

            /// \brief Initializes topology from input stream and validates with provided schema file.
            /// \throw runtime_error
            void init(std::istream& _stream, const std::string& _schemaFileName);

            /// \brief Get difference between THIS topology and a new one.
            /// \param[in] _topology New topology to calculate the difference with.
            /// \param[out] _removedTasks Tasks which exist in THIS topology and don't exist in new one.
            /// \param[out] _removedCollections Collections which exist in THIS topology and don't exist in new one.
            /// \param[out] _addedTasks Tasks which exist in new topology and don't exist in THIS one.
            /// \param[out] _addedCollections Collections which exist in new topology and don't exist in THIS one.
            void getDifference(CTopoCore& _topology,
                               IdSet_t& _removedTasks,
                               IdSet_t& _removedCollections,
                               IdSet_t& _addedTasks,
                               IdSet_t& _addedCollections);

            void setXMLValidationDisabled(bool _val);

            /// Accessors
            std::string getName() const;
            std::string getFilepath() const;
            uint32_t getHash() const;
            CTopoGroup::Ptr_t getMainGroup() const;
            /// \brief Returns runtime task by ID.
            const STopoRuntimeTask& getRuntimeTaskById(Id_t _id);
            /// \brief Returns runtime collection by ID.
            const STopoRuntimeCollection& getRuntimeCollectionById(Id_t _id);
            /// \brief Returns runtime task by runtime path.
            const STopoRuntimeTask& getRuntimeTaskByIdPath(const std::string& _idPath);
            /// \brief Returns runtime collection by runtime path.
            const STopoRuntimeCollection& getRuntimeCollectionByIdPath(const std::string& _idPath);
            /// \brief Returns runtime task by either ID or runtime path.
            const STopoRuntimeTask& getRuntimeTask(const std::string& _path);
            /// \brief Returns runtime collection by either ID or runtime path.
            const STopoRuntimeCollection& getRuntimeCollection(const std::string& _path);
            std::pair<size_t, size_t> getRequiredNofAgents(size_t _defaultNumSlots) const;
            size_t getTotalNofTasks() const;

            /// Iterators
            STopoRuntimeTask::FilterIteratorPair_t getRuntimeTaskIterator(
                const STopoRuntimeTask::Map_t& _map, STopoRuntimeTask::Condition_t _condition) const;
            STopoRuntimeTask::FilterIteratorPair_t getRuntimeTaskIterator(
                STopoRuntimeTask::Condition_t _condition = nullptr) const;
            STopoRuntimeCollection::FilterIteratorPair_t getRuntimeCollectionIterator(
                STopoRuntimeCollection::Condition_t _condition = nullptr) const;
            STopoRuntimeTask::FilterIteratorPair_t getRuntimeTaskIteratorForPropertyName(
                const std::string& _propertyName, Id_t _taskId) const;
            STopoRuntimeTask::FilterIteratorPair_t getRuntimeTaskIteratorMatchingPath(
                const std::string& _pathPattern) const;
            STopoRuntimeCollection::FilterIteratorPair_t getRuntimeCollectionIteratorMatchingPath(
                const std::string& _pathPattern) const;

            /// Accessors to internal data structures. Used for unit tests.
            const STopoRuntimeTask::Map_t& getIdToRuntimeTaskMap();
            const STopoRuntimeCollection::Map_t& getIdToRuntimeCollectionMap();
            const IdPathToIdMap_t& getTaskIdPathToIdMap();
            const IdPathToIdMap_t& getCollectionIdPathToIdMap();

            std::string stringOfTasks(const IdSet_t& _ids) const;
            std::string stringOfCollections(const IdSet_t& _ids) const;

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoCore& _topology);

          private:
            void FillTopoIndexToTopoElementMap(const CTopoElement::Ptr_t& _element);
            void FillIdToTopoElementMap(const CTopoElement::Ptr_t& _element);
            uint32_t CalculateHash(std::istream& _stream);
            Id_t calculateId(const std::string& _idPath, const std::string& _hashString);

          private:
            CTopoGroup::Ptr_t m_main{ nullptr }; ///< Main task group which we run

            STopoRuntimeTask::Map_t m_idToRuntimeTaskMap; ///< Task ID to runtime task object map
            STopoRuntimeCollection::Map_t
                m_idToRuntimeCollectionMap;      ///< Collection ID to runtime collection object map
            IdPathToIdMap_t m_taskIdPathToIdMap; ///< Task ID path to taskID map. Used for caching and fast search.
            IdPathToIdMap_t m_collectionIdPathToIdMap; ///< Collection ID path to collection ID map. Used for caching
                                                       ///< and fast search.
            std::map<std::string, size_t> m_counterMap;
            std::string m_currentCollectionIdPath;
            Id_t m_currentCollectionId{ 0 };

            bool m_bXMLValidationDisabled{ false }; ///< if true than XML will not be validated agains XSD
            std::string m_name;                     ///< Name of the topology
            uint32_t m_hash{ 0 };                   ///< CRC64 of the topology XML file
            std::string m_filepath;                 ///< Path to the XML topology file
            mutable std::mutex m_mtxTopoInit;       ///< Lock topo re-initialisation
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__Topology__) */
