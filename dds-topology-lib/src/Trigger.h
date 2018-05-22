// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__Trigger__
#define __DDS__Trigger__

// DDS
#include "TopoBase.h"
// STD
#include <string>

namespace dds
{
    namespace topology_api
    {
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

        /// \class CTrigger
        /// \brief Data class to hold task trigger.
        class CTrigger : public CTopoBase
        {
          public:
            /// \brief Constructor.
            CTrigger();

            /// \brief Destructor.
            virtual ~CTrigger();

            /// \brief Inherited from TopoBase
            void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

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
            friend std::ostream& operator<<(std::ostream& _strm, const CTrigger& _trigger);

          private:
            EActionType m_action;       ///< Action to be taken
            EConditionType m_condition; ///< Condition to fire the trigger
            std::string m_argument;     ///< Arguments string for action
        };

        typedef std::shared_ptr<CTrigger> TriggerPtr_t;
        typedef std::vector<TriggerPtr_t> TriggerPtrVector_t;
    } // namespace topology_api
} // namespace dds

#endif /* defined(__DDS__Trigger__) */
