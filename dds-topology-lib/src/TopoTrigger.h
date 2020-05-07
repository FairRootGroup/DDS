// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoTrigger__
#define __DDS__TopoTrigger__

// DDS
#include "TopoBase.h"
// STD
#include <string>

namespace dds
{
    namespace topology_api
    {
        /// \class CTrigger
        /// \brief Data class to hold task trigger.
        class CTopoTrigger : public CTopoBase
        {
          public:
            enum class EConditionType
            {
                None,
                TaskCrashed
            };

            enum class EActionType
            {
                None,
                RestartTask
            };

            using Ptr_t = std::shared_ptr<CTopoTrigger>;
            using PtrVector_t = std::vector<CTopoTrigger::Ptr_t>;

            /// \brief Constructor.
            CTopoTrigger(const std::string& _name);

            /// \brief Destructor.
            virtual ~CTopoTrigger();

            /// \brief Inherited from TopoBase
            void initFromPropertyTree(const boost::property_tree::ptree& _pt);

            /// \brief Inherited from TopoBase
            void saveToPropertyTree(boost::property_tree::ptree& _pt);

            EConditionType getCondition() const;
            EActionType getAction() const;
            const std::string& getArgument() const;

            void setAction(EActionType _action);
            void setCondition(EConditionType _condition);
            void setArgument(const std::string& _argument);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoTrigger& _trigger);

            /// \brief Inherited from TopoBase
            virtual std::string hashString() const;

          private:
            EActionType m_action{ EActionType::None };          ///< Action to be taken
            EConditionType m_condition{ EConditionType::None }; ///< Condition to fire the trigger
            std::string m_argument;                             ///< Arguments string for action
        };
    } // namespace topology_api
} // namespace dds

#endif /* defined(__DDS__TopoTrigger__) */
