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
#include <string>
// BOOST
#include <boost/iterator/filter_iterator.hpp>

namespace dds
{
    namespace topology_api
    {
        struct STaskInfo
        {
            STaskInfo()
                : m_task(nullptr)
                , m_taskIndex(0)
                , m_collectionIndex(std::numeric_limits<uint32_t>::max())
                , m_taskPath()
            {
            }
            TaskPtr_t m_task;
            size_t m_taskIndex;
            size_t m_collectionIndex;
            std::string m_taskPath;
        };

        class CTopology
        {
          public:
            /// Note that hash is of type uint_64.
            /// Hash is calculated using CRC64 algorithm.
            typedef std::map<uint64_t, STaskInfo> HashToTaskInfoMap_t;
            typedef std::function<bool(std::pair<uint64_t, const STaskInfo&>)> TaskInfoCondition_t;
            typedef boost::filter_iterator<TaskInfoCondition_t, HashToTaskInfoMap_t::const_iterator> TaskInfoIterator_t;
            typedef std::pair<TaskInfoIterator_t, TaskInfoIterator_t> TaskInfoIteratorPair_t;

            typedef std::map<uint64_t, TaskCollectionPtr_t> HashToTaskCollectionMap_t;
            typedef std::function<bool(std::pair<uint64_t, TaskCollectionPtr_t>)> TaskCollectionCondition_t;
            typedef boost::filter_iterator<TaskCollectionCondition_t, HashToTaskCollectionMap_t::const_iterator>
                TaskCollectionIterator_t;
            typedef std::pair<TaskCollectionIterator_t, TaskCollectionIterator_t> TaskCollectionIteratorPair_t;

            typedef std::map<std::string, TaskPtr_t> HashPathToTaskMap_t;
            typedef std::map<std::string, TaskCollectionPtr_t> HashPathToTaskCollectionMap_t;
            typedef std::map<CTopoIndex, TopoElementPtr_t, CompareTopoIndexLess> TopoIndexToTopoElementMap_t;
            typedef std::map<uint64_t, std::vector<uint64_t>> CollectionHashToTaskHashesMap_t;

            /// \brief Constructor.
            CTopology();

            /// \brief Destructor.
            virtual ~CTopology();

            /// \brief Initializes topology from specified file.
            /// \throw runtime_error
            void init(const std::string& _fileName, bool _initForTest = false);

            void setXMLValidationDisabled(bool _val);

            /// Accessors
            TaskGroupPtr_t getMainGroup() const;
            TopoElementPtr_t getTopoElementByTopoIndex(const CTopoIndex& _index) const;
            TaskPtr_t getTaskByHash(uint64_t _hash) const;
            const STaskInfo& getTaskInfoByHash(uint64_t _hash) const;
            TaskCollectionPtr_t getTaskCollectionByHash(uint64_t _hash) const;
            const std::vector<uint64_t>& getTaskHashesByTaskCollectionHash(uint64_t _hash) const;
            TaskPtr_t getTaskByHashPath(const std::string& _hashPath) const;

            /// Accessors to internal data structures. Used for unit tests.
            const TopoIndexToTopoElementMap_t& getTopoIndexToTopoElementMap() const;
            const HashToTaskInfoMap_t& getHashToTaskInfoMap() const;
            const HashToTaskCollectionMap_t& getHashToTaskCollectionMap() const;
            const HashPathToTaskMap_t& getHashPathToTaskMap() const;
            const HashPathToTaskCollectionMap_t& getHashPathToTaskCollectionMap() const;

            /// Iterators
            TaskInfoIteratorPair_t getTaskInfoIterator(TaskInfoCondition_t _condition = nullptr) const;
            TaskCollectionIteratorPair_t getTaskCollectionIterator(
                TaskCollectionCondition_t _condition = nullptr) const;
            TaskInfoIteratorPair_t getTaskInfoIteratorForPropertyId(const std::string& _propertyId) const;

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

            // HashToTaskMap_t m_hashToTaskMap;
            HashToTaskCollectionMap_t m_hashToTaskCollectionMap;
            HashToTaskInfoMap_t m_hashToTaskInfoMap;
            std::map<std::string, size_t> m_counterMap;
            std::string m_currentTaskCollectionHashPath;
            uint64_t m_currentTaskCollectionCrc;

            CollectionHashToTaskHashesMap_t m_collectionHashToTaskHashesMap;

            // FIXME: Hash path maps has to be removed due to performance reasons.
            // In any case we do not need them.
            // For the moment we store them only for tests.
            HashPathToTaskMap_t m_hashPathToTaskMap;
            HashPathToTaskCollectionMap_t m_hashPathToTaskCollectionMap;

            bool m_bXMLValidationDisabled; ///< if true than XML will not be validated agains XSD
        };
    }
}
#endif /* defined(__DDS__Topology__) */
