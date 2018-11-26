// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__Task__
#define __DDS__Task__

// DDS
#include "Requirement.h"
#include "TopoElement.h"
#include "TopoProperty.h"
#include "Trigger.h"
// STD
#include <memory>
#include <string>
// BOOST
#include <boost/iterator/filter_iterator.hpp>

namespace dds
{
    namespace topology_api
    {
        class CTask : public CTopoElement
        {
          public:
            /// \brief Constructor.
            CTask();

            /// \brief Destructor.
            virtual ~CTask();

            /// Accessors
            const std::string& getExe() const;
            const std::string& getEnv() const;
            bool isExeReachable() const;
            bool isEnvReachable() const;
            size_t getNofProperties() const;
            size_t getNofRequirements() const;
            size_t getNofTriggers() const;
            const TopoPropertyPtrMap_t& getProperties() const;
            const RequirementPtrVector_t& getRequirements() const;
            const TriggerPtrVector_t& getTriggers() const;
            /// Get property by ID. If property not fount than return nullptr.
            TopoPropertyPtr_t getProperty(const std::string& _id) const;

            /// Modifiers
            void setExe(const std::string& _exe);
            void setEnv(const std::string& _env);
            void setExeReachable(bool _exeReachable);
            void setEnvReachable(bool _envReachable);
            void setProperties(const TopoPropertyPtrMap_t& _properties);
            void addProperty(TopoPropertyPtr_t _property);
            void setRequirements(const RequirementPtrVector_t& _requirements);
            void addRequirement(RequirementPtr_t _requirement);
            void setTriggers(const TriggerPtrVector_t& _triggers);
            void addTrigger(TriggerPtr_t _trigger);

            // Parent collection and group ID
            std::string getParentCollectionId() const;
            std::string getParentGroupId() const;

            /// \brief If parent is a group than return N, else return 1.
            size_t getTotalCounter() const;

            /// \brief Inherited from DDSTopoElement.
            virtual size_t getNofTasks() const;

            /// \brief Inherited from DDSTopoElement.
            virtual size_t getTotalNofTasks() const;

            /// \brief Inherited from DDSTopoElement.
            virtual void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTask& _task);

          private:
            std::string m_exe;                     ///< Path to executable
            std::string m_env;                     ///< Path to environmtnt file
            bool m_exeReachable;                   ///< If executable is available on the WN
            bool m_envReachable;                   ///< If environment script is available on the WN
            TopoPropertyPtrMap_t m_properties;     ///< Properties
            RequirementPtrVector_t m_requirements; ///< Array of requirements
            TriggerPtrVector_t m_triggers;         ///< Array of triggers
        };

        typedef std::shared_ptr<CTask> TaskPtr_t;
        typedef std::vector<TaskPtr_t> TaskPtrVector_t;

        struct STaskInfo
        {
            STaskInfo()
                : m_task(nullptr)
                , m_taskIndex(0)
                , m_collectionIndex(std::numeric_limits<uint32_t>::max())
                , m_taskPath()
                , m_taskCollectionHash(0)
            {
            }
            TaskPtr_t m_task;
            size_t m_taskIndex;
            size_t m_collectionIndex;
            std::string m_taskPath;
            uint64_t m_taskCollectionHash;
        };

        typedef std::map<uint64_t, STaskInfo> HashToTaskInfoMap_t;
        typedef std::function<bool(std::pair<uint64_t, const STaskInfo&>)> TaskInfoCondition_t;
        typedef boost::filter_iterator<TaskInfoCondition_t, HashToTaskInfoMap_t::const_iterator> TaskInfoIterator_t;
        typedef std::pair<TaskInfoIterator_t, TaskInfoIterator_t> TaskInfoIteratorPair_t;
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__Task__) */
