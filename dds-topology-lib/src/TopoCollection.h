// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoCollection__
#define __DDS__TopoCollection__

// DDS
#include "TopoContainer.h"
#include "TopoDef.h"
#include "TopoRequirement.h"
#include "TopoTask.h"

namespace dds
{
    namespace topology_api
    {
        class CTopoCollection : public CTopoContainer
        {
          public:
            using Ptr_t = std::shared_ptr<CTopoCollection>;
            using PtrVector_t = std::vector<CTopoCollection::Ptr_t>;

            /// \brief Constructor.
            CTopoCollection(const std::string& _name);

            /// \brief Destructor.
            virtual ~CTopoCollection();

            /// \brief Inherited from TopoElement.
            virtual size_t getNofTasks() const;

            /// \brief Inherited from TopoElement.
            virtual size_t getTotalNofTasks() const;

            /// \brief Inherited from TopoElement.
            void initFromPropertyTree(const boost::property_tree::ptree& _pt);

            /// \brief Inherited from TopoBase
            void saveToPropertyTree(boost::property_tree::ptree& _pt);

            /// \brief If parent is a group than return N, else return 1.
            size_t getTotalCounter() const;

            size_t getNofRequirements() const;
            const CTopoRequirement::PtrVector_t& getRequirements() const;
            CTopoRequirement::Ptr_t addRequirement(const std::string& _name);

            /// \brief Inherited from TopoBase
            virtual std::string hashString() const;

          private:
            CTopoRequirement::PtrVector_t m_requirements; ///< Array of requirement
        };

        struct STopoRuntimeCollection
        {
            typedef std::map<Id_t, STopoRuntimeCollection> Map_t;
            typedef std::function<bool(std::pair<Id_t, const STopoRuntimeCollection&>)> Condition_t;
            typedef boost::filter_iterator<STopoRuntimeCollection::Condition_t,
                                           STopoRuntimeCollection::Map_t::const_iterator>
                FilterIterator_t;
            typedef std::pair<STopoRuntimeCollection::FilterIterator_t, STopoRuntimeCollection::FilterIterator_t>
                FilterIteratorPair_t;

            CTopoCollection::Ptr_t m_collection{ nullptr };
            Id_t m_collectionId{ 0 };
            size_t m_collectionIndex{ 0 };
            std::string m_collectionPath;
            STopoRuntimeTask::Map_t m_idToRuntimeTaskMap; ///< Map of task ID to STopoRuntimeTask
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoCollection__) */
