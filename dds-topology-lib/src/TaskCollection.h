// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TaskCollection__
#define __DDS__TaskCollection__

// DDS
#include "Requirement.h"
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

            RequirementPtr_t getRequirement() const;
            void setRequirement(RequirementPtr_t _requirement);

          private:
            RequirementPtr_t m_requirement; ///> Requirement
        };

        typedef std::shared_ptr<CTaskCollection> TaskCollectionPtr_t;
        typedef std::vector<TaskCollectionPtr_t> TaskCollectionPtrVector_t;
    }
}
#endif /* defined(__DDS__Topology__) */
