// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__Topology__
#define __DDS__Topology__

// DDS Topo
#include "TopoIndex.h"
#include "TaskGroup.h"
#include "TaskCollection.h"
#include "TopoElement.h"
#include "Task.h"
// STD
#include <ostream>
#include <string>
#include <map>
// BOOST
#include <boost/iterator/filter_iterator.hpp>

namespace dds
{
    class CTopology
    {
      public:
        /// Note that hash is of type uint_64.
        /// Hash is calculated using CRC64 algorithm.
        typedef std::map<uint64_t, TaskPtr_t> HashToTaskMap_t;
        typedef std::map<uint64_t, TaskCollectionPtr_t> HashToTaskCollectionMap_t;
        typedef std::function<bool(std::pair<uint64_t, TaskPtr_t>)> TaskCondition_t;
        typedef std::function<bool(std::pair<uint64_t, TaskCollectionPtr_t>)> TaskCollectionCondition_t;
        typedef boost::filter_iterator<TaskCondition_t, HashToTaskMap_t::const_iterator> TaskIterator_t;
        typedef std::pair<TaskIterator_t, TaskIterator_t> TaskIteratorPair_t;
        typedef boost::filter_iterator<TaskCollectionCondition_t, HashToTaskCollectionMap_t::const_iterator>
            TaskCollectionIterator_t;
        typedef std::pair<TaskCollectionIterator_t, TaskCollectionIterator_t> TaskCollectionIteratorPair_t;
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

        /// Accessors
        TaskGroupPtr_t getMainGroup() const;
        TopoElementPtr_t getTopoElementByTopoIndex(const CTopoIndex& _index) const;
        TaskPtr_t getTaskByHash(uint64_t _hash) const;
        TaskCollectionPtr_t getTaskCollectionByHash(uint64_t _hash) const;

        /// Accessors to internal data structures. Used for unit tests.
        const TopoIndexToTopoElementMap_t getTopoIndexToTopoElementMap() const;
        const HashToTaskMap_t getHashToTaskMap() const;
        const HashToTaskCollectionMap_t getHashToTaskCollectionMap() const;
        const HashPathToTaskMap_t& getHashPathToTaskMap() const;
        const HashPathToTaskCollectionMap_t& getHashPathToTaskCollectionMap() const;

        /// Iterators
        TaskIteratorPair_t getTaskIterator(TaskCondition_t _condition = nullptr) const;
        TaskCollectionIteratorPair_t getTaskCollectionIterator(TaskCollectionCondition_t _condition = nullptr) const;

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

        TaskGroupPtr_t m_main; ///> Main task group which we run

        TopoIndexToTopoElementMap_t m_topoIndexToTopoElementMap;

        HashToTaskMap_t m_hashToTaskMap;
        HashToTaskCollectionMap_t m_hashToTaskCollectionMap;
        std::map<std::string, size_t> m_counterMap;
        std::string m_currentTaskCollectionHashPath;

        // FIXME: Hash path maps has to be removed due to performance reasons.
        // In any case we do not need them.
        // For the moment we store them only for tests.
        HashPathToTaskMap_t m_hashPathToTaskMap;
        HashPathToTaskCollectionMap_t m_hashPathToTaskCollectionMap;
    };
}
#endif /* defined(__DDS__Topology__) */
