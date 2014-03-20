// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopoBase__
#define __DDS__DDSTopoBase__

// STD
#include <sstream>
#include <string>
#include <vector>
// BOOST
#include <boost/property_tree/ptree.hpp>

enum class DDSTopoType
{
    TOPO_BASE,
    TOPO_PROPERTY,
    TOPO_ELEMENT,
    TASK,
    COLLECTION,
    GROUP,
    PORT
};

class DDSTopoBase
{
  public:
    /// Modifiers
    void setName(const std::string& _name);

    /// Accessors
    std::string getName() const;
    DDSTopoType getType() const;

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
    static const boost::property_tree::ptree& findElement(DDSTopoType _type, const std::string& _name, const boost::property_tree::ptree& _pt);

    /// \brief Returns string representation of an object.
    /// \return String representation of an object.
    virtual std::string toString() const;

    /// \brief Operator << for convenient output to ostream.
    /// \return Insertion stream in order to be able to call a succession of
    /// insertion operations.
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTopoBase& _element);

  protected:
    /// \brief Constructor.
    DDSTopoBase();

    /// \brief Destructor.
    virtual ~DDSTopoBase();

    void setType(DDSTopoType _type);

  private:
    std::string m_name; ///> Name of topology element
    DDSTopoType m_type; ///> Type of the topology element

    // FIXME: Probably we have to add an ID which will uniquely identifies the object.
};

typedef std::shared_ptr<DDSTopoBase> DDSTopoBasePtr_t;
typedef std::vector<DDSTopoBasePtr_t> DDSTopoBasePtrVector_t;

#endif /* defined(__DDS__DDSTopoBase__) */
