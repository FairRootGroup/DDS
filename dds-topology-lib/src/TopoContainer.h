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

            /// \brief Adds TopoElement to a container using enum type.
            /// \param[in] _type Type of the element to be added to the container (Task, Collection or Group).
            /// \throw std::runtime_error if element can't be added.
            CTopoElement::Ptr_t addElement(CTopoBase::EType _type, const std::string& _name);

            /// \brief Adds TopoElement to a container using specified type. CTopoTask, CTopoCollection and CTopoGroup
            /// are supported. \throw std::runtime_error if element can't be added.
            template <class Object_t>
            typename Object_t::Ptr_t addElement(const std::string& _name);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoContainer& _taskContainer);

          protected:
            /// \brief Constructor.
            CTopoContainer(const std::string& _name);

            /// \brief Destructor.
            virtual ~CTopoContainer();

            /// \brief Default implementation for TopoElement::getNofTasks, TopoElement::getTotalNofTasks.
            /// Calculate recursively number of tasks in all daughter elements.
            size_t getNofTasksDefault() const;

            /// \brief Checks if element can be added to the container.
            /// TopoTask can be added to any container (TopoCollection or TopoGroup).
            /// TopoCollection can be added only to TopoGroup.
            /// TopoGroup can be added only to Main TopoGroup.
            /// \param[in] _type Type of the element to be added to the container.
            /// \return True if TopoElement can be added, otherwise return false.
            bool canAddElement(CTopoBase::EType _type);

            /// \brief Makes new topology element.
            /// \throw std::runtime_error if element can't be added.
            template <class Object_t>
            typename Object_t::Ptr_t makeElement(const std::string& _name)
            {
                auto element{ std::make_shared<Object_t>(_name) };
                element->setParent(this);
                element->setNameToValueCache(getNameToValueCache());
                m_elements.push_back(element);
                return element;
            }

          private:
            CTopoElement::PtrVector_t m_elements; ///< Vector of topology elements in collection.
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoContainer__) */
