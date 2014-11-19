// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoBase__
#define __DDS__TopoBase__

// DDS
#include "Index.h"
// STD
#include <sstream>
#include <string>
#include <vector>
// BOOST
#include <boost/property_tree/ptree.hpp>

namespace dds
{
    enum class ETopoType
    {
        TOPO_BASE,
        TOPO_PROPERTY,
        TOPO_ELEMENT,
        TASK,
        COLLECTION,
        GROUP,
        REQUIREMENT
    };

    class CTopoBase
    {
      public:
        /// Modifiers
        void setId(const std::string& _id);
        void setParent(CTopoBase* _parent);

        /// Accessors
        std::string getId() const;
        ETopoType getType() const;
        CTopoBase* getParent() const;

        /// \brief Return full path to topo element or property.
        std::string getPath() const;

        /// \brief Return index of topo element or property.
        CIndex getIndex() const;

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
        static const boost::property_tree::ptree& findElement(ETopoType _type,
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

        void setType(ETopoType _type);

      private:
        std::string m_id;    ///> Identificator of topology element
        ETopoType m_type;    ///> Type of the topology element
        CTopoBase* m_parent; ///> Pointer to parent element

        // FIXME: Probably we have to add an ID which will uniquely identifies the object.
    };

    typedef std::shared_ptr<CTopoBase> TopoBasePtr_t;
    typedef std::vector<TopoBasePtr_t> TopoBasePtrVector_t;
}
#endif /* defined(__DDS__TopoBase__) */
