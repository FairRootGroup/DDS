// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoBase__
#define __DDS__TopoBase__

// STD
#include <sstream>
#include <string>
#include <vector>
// BOOST
#include <boost/property_tree/ptree.hpp>

namespace dds
{
    namespace topology_api
    {
        class CTopoBase
        {
          public:
            enum class EType
            {
                TOPO_BASE,
                TOPO_PROPERTY,
                TOPO_ELEMENT,
                TASK,
                COLLECTION,
                GROUP,
                REQUIREMENT,
                TOPO_VARS,
                TRIGGER
            };

            typedef std::shared_ptr<CTopoBase> Ptr_t;
            typedef std::vector<CTopoBase::Ptr_t> PtrVector_t;

          public:
            /// Modifiers
            void setId(const std::string& _id);
            void setParent(CTopoBase* _parent);

            /// Accessors
            std::string getId() const;
            CTopoBase::EType getType() const;
            CTopoBase* getParent() const;

            /// \brief Return full path to topo element or property.
            std::string getPath() const;

            /// \brief Initialize object with data from property tree.
            /// \param[in] _name Name of the object as in input file.
            /// \param[in] _pt Property tree starting from root.
            virtual void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt) = 0;

            /// \brief Helper function to find element in property tree by type and name.
            /// \param[in] _type Type of the topo element we are looking for.
            /// \param[in] _name Name of element we are looking for.
            /// \param[in] _pt Property tree.
            /// \return Property tree with root node pointing to found element.
            /// \throw logic_error if element was not found.
            /// \note This function does not catch exceptions from property tree.
            static const boost::property_tree::ptree& findElement(CTopoBase::EType _type,
                                                                  const std::string& _name,
                                                                  const boost::property_tree::ptree& _pt);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoBase& _element);

          protected:
            /// \brief Constructor.
            CTopoBase();

            /// \brief Destructor.
            virtual ~CTopoBase();

            void setType(CTopoBase::EType _type);

          private:
            std::string m_id;        ///< Identificator of topology element
            CTopoBase::EType m_type; ///< Type of the topology element
            CTopoBase* m_parent;     ///< Pointer to parent element
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoBase__) */
