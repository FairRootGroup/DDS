// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoTask__
#define __DDS__TopoTask__

// DDS
#include "TopoDef.h"
#include "TopoElement.h"
#include "TopoProperty.h"
#include "TopoRequirement.h"
#include "TopoTrigger.h"
// STD
#include <memory>
#include <string>
// BOOST
#include <boost/iterator/filter_iterator.hpp>

namespace dds
{
    namespace topology_api
    {
        class CTopoTask : public CTopoElement
        {
          public:
            using Ptr_t = std::shared_ptr<CTopoTask>;
            using PtrVector_t = std::vector<CTopoTask::Ptr_t>;

            /// \brief Constructor.
            CTopoTask(const std::string& _name);

            /// \brief Destructor.
            virtual ~CTopoTask();

            /// Accessors
            const std::string& getExe() const;
            const std::string& getEnv() const;
            bool isExeReachable() const;
            bool isEnvReachable() const;
            size_t getNofProperties() const;
            size_t getNofRequirements() const;
            size_t getNofTriggers() const;
            const CTopoProperty::PtrMap_t& getProperties() const;
            const CTopoRequirement::PtrVector_t& getRequirements() const;
            const CTopoTrigger::PtrVector_t& getTriggers() const;
            /// Get property by ID. If property not fount than return nullptr.
            CTopoProperty::Ptr_t getProperty(const std::string& _id) const;

            /// Modifiers
            void setExe(const std::string& _exe);
            void setEnv(const std::string& _env);
            void setExeReachable(bool _exeReachable);
            void setEnvReachable(bool _envReachable);
            CTopoProperty::Ptr_t addProperty(const std::string& _name);
            CTopoRequirement::Ptr_t addRequirement(const std::string& _name);
            CTopoTrigger::Ptr_t addTrigger(const std::string& _name);

            // Parent collection and group ID
            std::string getParentCollectionId() const;
            std::string getParentGroupId() const;

            /// \brief If parent is a group than return N, else return 1.
            size_t getTotalCounter() const;

            /// \brief Inherited from DDSTopoElement.
            virtual size_t getNofTasks() const;

            /// \brief Inherited from DDSTopoElement.
            virtual size_t getTotalNofTasks() const;

            /// \brief Inherited from TopoElement.
            virtual void initFromPropertyTree(const boost::property_tree::ptree& _pt);

            /// \brief Inherited from TopoElement.
            virtual void saveToPropertyTree(boost::property_tree::ptree& _pt);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoTask& _task);

            /// \brief Inherited from TopoBase
            virtual std::string hashString() const;

          private:
            std::string m_exe;                            ///< Path to executable
            std::string m_env;                            ///< Path to environmtnt file
            bool m_exeReachable{ true };                  ///< If executable is available on the WN
            bool m_envReachable{ true };                  ///< If environment script is available on the WN
            CTopoProperty::PtrMap_t m_properties;         ///< Properties
            CTopoRequirement::PtrVector_t m_requirements; ///< Array of requirements
            CTopoTrigger::PtrVector_t m_triggers;         ///< Array of triggers
        };

        struct STopoRuntimeTask
        {
            typedef std::map<Id_t, STopoRuntimeTask> Map_t;
            typedef std::function<bool(std::pair<Id_t, const STopoRuntimeTask&>)> Condition_t;
            typedef boost::filter_iterator<STopoRuntimeTask::Condition_t, STopoRuntimeTask::Map_t::const_iterator>
                FilterIterator_t;
            typedef std::pair<STopoRuntimeTask::FilterIterator_t, STopoRuntimeTask::FilterIterator_t>
                FilterIteratorPair_t;

            CTopoTask::Ptr_t m_task{ nullptr };
            Id_t m_taskId{ 0 };
            size_t m_taskIndex{ 0 };
            size_t m_collectionIndex{ std::numeric_limits<uint32_t>::max() };
            std::string m_taskPath;
            Id_t m_taskCollectionId{ 0 };
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoTask__) */
