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
        typedef std::map<size_t, TaskPtr_t> IndexToTaskMap_t;
        typedef std::map<size_t, TaskCollectionPtr_t> IndexToTaskCollectionMap_t;
        typedef std::function<bool(std::pair<size_t, TaskPtr_t>)> TaskCondition_t;
        typedef std::function<bool(std::pair<size_t, TaskCollectionPtr_t>)> TaskCollectionCondition_t;
        typedef boost::filter_iterator<TaskCondition_t, IndexToTaskMap_t::const_iterator> TaskIterator_t;
        typedef std::pair<TaskIterator_t, TaskIterator_t> TaskIteratorPair_t;
        typedef boost::filter_iterator<TaskCollectionCondition_t, IndexToTaskCollectionMap_t::const_iterator>
            TaskCollectionIterator_t;
        typedef std::pair<TaskCollectionIterator_t, TaskCollectionIterator_t> TaskCollectionIteratorPair_t;

        /// \brief Constructor.
        CTopology();

        /// \brief Destructor.
        virtual ~CTopology();

        /// \brief Initializes topology from specified file.
        /// \throw runtime_error
        void init(const std::string& _fileName);

        /// Accessors
        TaskGroupPtr_t getMainGroup() const;
        TopoElementPtr_t getTopoElementByTopoIndex(const CTopoIndex& _index) const;
        TaskPtr_t getTaskByIndex(size_t _index) const;
        TaskCollectionPtr_t getTaskCollectionByIndex(size_t _index) const;

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
        void FillIndexToTopoElementMap(const TopoElementPtr_t& _element);

        TaskGroupPtr_t m_main; ///> Main task group which we run

        typedef std::map<CTopoIndex, TopoElementPtr_t, CompareTopoIndexLess> TopoIndexToTopoElementMap_t;
        TopoIndexToTopoElementMap_t m_topoIndexToTopoElementMap;

        IndexToTaskMap_t m_indexToTaskMap;
        IndexToTaskCollectionMap_t m_indexToTaskCollectionMap;
        size_t m_taskCounter;
        size_t m_taskCollectionCounter;
    };
}
#endif /* defined(__DDS__Topology__) */
