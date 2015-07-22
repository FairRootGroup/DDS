// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef DDS_TaskGroup_h
#define DDS_TaskGroup_h
// DDS
#include "TaskContainer.h"
#include "TopoIndex.h"

namespace dds
{
    namespace topology_api
    {
        class CTaskGroup : public CTaskContainer
        {
          public:
            /// \brief Constructor.
            CTaskGroup();

            /// \brief Destructor.
            virtual ~CTaskGroup();

            /// \brief Inherited from TopoElement.
            virtual size_t getNofTasks() const;

            /// \brief Inherited from TopoElement.
            virtual size_t getTotalNofTasks() const;

            /// \brief Inherited from TopoBase.
            void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

            size_t getN() const;

            void setN(size_t _n);

            TopoElementPtrVector_t getElementsByType(ETopoType _type) const;

            TopoIndexVector_t getTopoIndicesByType(ETopoType _type) const;

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTaskGroup& _taskContainer);

          private:
            size_t m_n; ///> Number of times this group has to be executed
        };

        typedef std::shared_ptr<CTaskGroup> TaskGroupPtr_t;
        // typedef std::vector<CTaskGroupPtr_t> TaskGroupPtrVector_t;
    }
}
#endif
