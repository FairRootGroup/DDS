// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoContainer__
#define __DDS__TopoContainer__

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
        class CTopoContainer : public CTopoElement
        {
          public:
            typedef std::shared_ptr<CTopoContainer> Ptr_t;
            typedef std::vector<CTopoContainer::Ptr_t> PtrVector_t;

          public:
            /// \brief Return number of elements.
            /// \return Number of elements.
            size_t getNofElements() const;

            /// \brief Return topology element by index.
            /// \return Topology element by index.
            /// \throw std::out_of_range
            CTopoElement::Ptr_t getElement(size_t _i) const;

            /// \brief Return vector of elements.
            /// \return Vector of elements.
            const CTopoElement::PtrVector_t& getElements() const;

            void setElements(const CTopoElement::PtrVector_t& _elements);

            void addElement(CTopoElement::Ptr_t _element);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoContainer& _taskContainer);

          protected:
            /// \brief Constructor.
            CTopoContainer();

            /// \brief Destructor.
            virtual ~CTopoContainer();

            /// \brief Default implementation for TopoElement::getNofTasks, TopoElement::getTotalNofTasks.
            /// Calculate recursively number of tasks in all daughter elements.
            size_t getNofTasksDefault() const;

          private:
            CTopoElement::PtrVector_t m_elements; ///< Vector of topology elements in collection.
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoContainer__) */
