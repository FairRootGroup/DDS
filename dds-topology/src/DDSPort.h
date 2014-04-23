// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSPort__
#define __DDS__DDSPort__
// DDS
#include "DDSTopoProperty.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
// STL
#include <string>
#include <vector>

enum class DDSPortType
{
    SERVER,
    CLIENT
};

class DDSPort : public DDSTopoProperty
{
  public:
    typedef std::pair<unsigned short, unsigned short> DDSPortRange_t;

    /// \brief Constructor.
    DDSPort();

    /// \brief Destructor.
    virtual ~DDSPort();

  public:
    /// Accessors
    void setRange(unsigned short _min, unsigned short _max);
    void setPortType(DDSPortType _portType);

    /// Modifiers
    const DDSPortRange_t& getRange() const;
    DDSPortType getPortType() const;

    /// \brief Inherited from DDSTopoBase
    void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

    /// \brief Returns string representation of an object.
    /// \return String representation of an object.
    virtual std::string toString() const;

    /// \brief Operator << for convenient output to ostream.
    /// \return Insertion stream in order to be able to call a succession of
    /// insertion operations.
    friend std::ostream& operator<<(std::ostream& _strm, const DDSPort& _port);

  private:
    DDSPortRange_t m_range; ///> Requested range of ports.
    DDSPortType m_portType; ///> Port type: server or client
};

typedef std::shared_ptr<DDSPort> DDSPortPtr_t;
typedef std::vector<DDSPortPtr_t> DDSPortPtrVector_t;

#endif /* defined(__DDS__DDSPort__) */
