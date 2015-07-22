// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TaskContainer__
#define __DDS__TaskContainer__

// DDS
#include "TopoElement.h"
// STD
#include <iostream>
#include <string>
#include <vector>

namespace dds
{
    namespace topology_api
    {
        class CTaskContainer : public CTopoElement
        {
          public:
            /// \brief Return number of elements.
            /// \return Number of elements.
            size_t getNofElements() const;

            /// \brief Return topology element by index.
            /// \return Topology element by index.
            /// \throw std::out_of_range
            TopoElementPtr_t getElement(size_t _i) const;

            /// \brief Return vector of elements.
            /// \return Vector of elements.
            const TopoElementPtrVector_t& getElements() const;

            void setElements(const TopoElementPtrVector_t& _elements);

            void addElement(TopoElementPtr_t _element);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTaskContainer& _taskContainer);

          protected:
            /// \brief Constructor.
            CTaskContainer();

            /// \brief Destructor.
            virtual ~CTaskContainer();

            /// \brief Default implementation for TopoElement::getNofTasks, TopoElement::getTotalNofTasks.
            /// Calculate recursively number of tasks in all daughter elements.
            /// \param multiplicator Multiplication factor for all daughter nodes.
            size_t getNofTasksDefault() const;

          private:
            TopoElementPtrVector_t m_elements; ///> Vector of topology elements in collection.
        };

        typedef std::shared_ptr<CTaskContainer> TaskContainerPtr_t;
        typedef std::vector<TaskContainerPtr_t> TaskContainerPtrVector_t;
    }
}
#endif /* defined(__DDS__TaskContainer__) */
