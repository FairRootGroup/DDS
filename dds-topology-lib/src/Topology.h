// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__Topology__
#define __DDS__Topology__

// DDS Topo
#include "TopoCollection.h"
#include "TopoElement.h"
#include "TopoGroup.h"
#include "TopoTask.h"
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

          public:
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
            CTopoGroup::Ptr_t getMainGroup() const;
            const STopoRuntimeTask& getRuntimeTaskByHash(uint64_t _hash) const;
            const STopoRuntimeCollection& getRuntimeCollectionByHash(uint64_t _hash) const;
            const STopoRuntimeTask& getRuntimeTaskByHashPath(const std::string& _hashPath) const;
            const STopoRuntimeCollection& getRuntimeCollectionByHashPath(const std::string& _hashPath) const;

            /// Iterators
            STopoRuntimeTask::FilterIteratorPair_t getRuntimeTaskIterator(
                const STopoRuntimeTask::Map_t& _map, STopoRuntimeTask::Condition_t _condition) const;
            STopoRuntimeTask::FilterIteratorPair_t getRuntimeTaskIterator(
                STopoRuntimeTask::Condition_t _condition = nullptr) const;
            STopoRuntimeCollection::FilterIteratorPair_t getRuntimeCollectionIterator(
                STopoRuntimeCollection::Condition_t _condition = nullptr) const;
            STopoRuntimeTask::FilterIteratorPair_t getRuntimeTaskIteratorForPropertyId(const std::string& _propertyId,
                                                                                       uint64_t _taskHash) const;

            /// Accessors to internal data structures. Used for unit tests.
            const STopoRuntimeTask::Map_t& getHashToRuntimeTaskMap() const;
            const STopoRuntimeCollection::Map_t& getHashToRuntimeCollectionMap() const;

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
            void FillTopoIndexToTopoElementMap(const CTopoElement::Ptr_t& _element);
            void FillHashToTopoElementMap(const CTopoElement::Ptr_t& _element);

            CTopoGroup::Ptr_t m_main; ///< Main task group which we run

            STopoRuntimeTask::Map_t m_hashToRuntimeTaskMap;
            STopoRuntimeCollection::Map_t m_hashToRuntimeCollectionMap;
            std::map<std::string, size_t> m_counterMap;
            std::string m_currentCollectionHashPath;
            uint64_t m_currentCollectionCrc;

            bool m_bXMLValidationDisabled; ///< if true than XML will not be validated agains XSD
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__Topology__) */
