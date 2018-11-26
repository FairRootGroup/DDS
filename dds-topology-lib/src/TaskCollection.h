// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TaskCollection__
#define __DDS__TaskCollection__

// DDS
#include "Requirement.h"
#include "Task.h"
#include "TaskContainer.h"

namespace dds
{
    namespace topology_api
    {
        class CTaskCollection : public CTaskContainer
        {
          public:
            /// \brief Constructor.
            CTaskCollection();

            /// \brief Destructor.
            virtual ~CTaskCollection();

            /// \brief Inherited from TopoElement.
            virtual size_t getNofTasks() const;

            /// \brief Inherited from TopoElement.
            virtual size_t getTotalNofTasks() const;

            /// \brief Inherited from TopoElement.
            void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

            /// \brief If parent is a group than return N, else return 1.
            size_t getTotalCounter() const;

            size_t getNofRequirements() const;
            const RequirementPtrVector_t& getRequirements() const;
            void setRequirement(const RequirementPtrVector_t& _requirements);
            void addRequirement(RequirementPtr_t _requirement);

          private:
            RequirementPtrVector_t m_requirements; ///< Array of requirement
        };

        typedef std::shared_ptr<CTaskCollection> TaskCollectionPtr_t;
        typedef std::vector<TaskCollectionPtr_t> TaskCollectionPtrVector_t;

        struct STaskCollectionInfo
        {
            STaskCollectionInfo()
                : m_collection(nullptr)
                , m_collectionIndex(0)
                , m_collectionPath()
            {
            }
            TaskCollectionPtr_t m_collection;
            size_t m_collectionIndex;
            std::string m_collectionPath;
            HashToTaskInfoMap_t m_hashToTaskInfoMap; ///< Map of task ID to CTaskInfo
        };
        typedef std::map<uint64_t, STaskCollectionInfo> HashToTaskCollectionInfoMap_t;
        typedef std::function<bool(std::pair<uint64_t, const STaskCollectionInfo&>)> TaskCollectionInfoCondition_t;
        typedef boost::filter_iterator<TaskCollectionInfoCondition_t, HashToTaskCollectionInfoMap_t::const_iterator>
            TaskCollectionInfoIterator_t;
        typedef std::pair<TaskCollectionInfoIterator_t, TaskCollectionInfoIterator_t> TaskCollectionInfoIteratorPair_t;
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__Topology__) */
