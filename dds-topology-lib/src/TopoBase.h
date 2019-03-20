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
            void setName(const std::string& _name);
            void setParent(CTopoBase* _parent);

            /// Accessors
            std::string getName() const;
            CTopoBase::EType getType() const;
            CTopoBase* getParent() const;

            /// \brief Return full path to topo element or property.
            std::string getPath() const;

            /// \brief Initialize object with data from property tree.
            /// \param[in] _name Name of the object as in input file.
            /// \param[in] _pt Property tree starting from root.
            virtual void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt) = 0;

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
            std::string m_name;      ///< Name of topology element
            CTopoBase::EType m_type; ///< Type of the topology element
            CTopoBase* m_parent;     ///< Pointer to parent element
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoBase__) */
