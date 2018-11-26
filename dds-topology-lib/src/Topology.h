// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__Topology__
#define __DDS__Topology__

// DDS Topo
#include "Task.h"
#include "TaskCollection.h"
#include "TaskGroup.h"
#include "TopoElement.h"
#include "TopoIndex.h"
// STD
#include <map>
#include <ostream>
#include <set>
#include <string>

namespace dds
{
    namespace topology_api
    {
        class CTopology
        {
          public:
            /// Note that hash is of type uint_64.
            /// Hash is calculated using CRC64 algorithm.
            typedef std::set<uint64_t> HashSet_t;

            typedef std::map<std::string, TaskPtr_t> HashPathToTaskMap_t;
            typedef std::map<std::string, TaskCollectionPtr_t> HashPathToTaskCollectionMap_t;
            typedef std::map<CTopoIndex, TopoElementPtr_t, CompareTopoIndexLess> TopoIndexToTopoElementMap_t;

            /// \brief Constructor.
            CTopology();

            /// \brief Destructor.
            virtual ~CTopology();

            /// \brief Initializes topology from specified file.
            /// \throw runtime_error
            void init(const std::string& _fileName, bool _initForTest = false);

            /// \brief Get difference between THIS topology and a new one.
            /// \param[in] _topology New topology to calculate the difference with.
            /// \param[out] _removedTasks Tasks which exist in THIS topology and don't exist in new one.
            /// \param[out] _removedCollections Collections which exist in THIS topology and don't exist in new one.
            /// \param[out] _addedTasks Tasks which exist in new topology and don't exist in THIS one.
            /// \param[out] _addedCollections Collections which exist in new topology and don't exist in THIS one.
            void getDifference(const CTopology& _topology,
                               HashSet_t& _removedTasks,
                               HashSet_t& _removedCollections,
                               HashSet_t& _addedTasks,
                               HashSet_t& _addedCollections);

            void setXMLValidationDisabled(bool _val);

            /// Accessors
            TaskGroupPtr_t getMainGroup() const;
            TopoElementPtr_t getTopoElementByTopoIndex(const CTopoIndex& _index) const;
            TaskPtr_t getTaskByHash(uint64_t _hash) const;
            const STaskInfo& getTaskInfoByHash(uint64_t _hash) const;
            const STaskCollectionInfo& getTaskCollectionInfoByHash(uint64_t _hash) const;
            TaskPtr_t getTaskByHashPath(const std::string& _hashPath) const;

            /// Accessors to internal data structures. Used for unit tests.
            const TopoIndexToTopoElementMap_t& getTopoIndexToTopoElementMap() const;
            const HashToTaskInfoMap_t& getHashToTaskInfoMap() const;
            const HashToTaskCollectionInfoMap_t& getHashToTaskCollectionInfoMap() const;
            const HashPathToTaskMap_t& getHashPathToTaskMap() const;
            const HashPathToTaskCollectionMap_t& getHashPathToTaskCollectionMap() const;

            /// Iterators
            TaskInfoIteratorPair_t getTaskInfoIterator(const HashToTaskInfoMap_t& _map,
                                                       TaskInfoCondition_t _condition) const;
            TaskInfoIteratorPair_t getTaskInfoIterator(TaskInfoCondition_t _condition = nullptr) const;
            TaskCollectionInfoIteratorPair_t getTaskCollectionInfoIterator(
                TaskCollectionInfoCondition_t _condition = nullptr) const;
            TaskInfoIteratorPair_t getTaskInfoIteratorForPropertyId(const std::string& _propertyId,
                                                                    uint64_t _taskHash) const;

            std::string stringOfTasks(const HashSet_t& _ids) const;
            std::string stringOfCollections(const HashSet_t& _ids) const;

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopology& _topology);

          private:
            void FillTopoIndexToTopoElementMap(const TopoElementPtr_t& _element);
            void FillHashToTopoElementMap(const TopoElementPtr_t& _element, bool _fillHashPathMaps = false);

            uint64_t getNextHashForTask(uint64_t _crc) const;
            uint64_t getNextHashForTaskCollection(uint64_t _crc) const;

            TaskGroupPtr_t m_main; ///< Main task group which we run

            TopoIndexToTopoElementMap_t m_topoIndexToTopoElementMap;

            HashToTaskInfoMap_t m_hashToTaskInfoMap;
            HashToTaskCollectionInfoMap_t m_hashToTaskCollectionInfoMap;
            std::map<std::string, size_t> m_counterMap;
            std::string m_currentTaskCollectionHashPath;
            uint64_t m_currentTaskCollectionCrc;

            // FIXME: Hash path maps has to be removed due to performance reasons.
            // In any case we do not need them.
            // For the moment we store them only for tests.
            HashPathToTaskMap_t m_hashPathToTaskMap;
            HashPathToTaskCollectionMap_t m_hashPathToTaskCollectionMap;

            bool m_bXMLValidationDisabled; ///< if true than XML will not be validated agains XSD
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__Topology__) */
