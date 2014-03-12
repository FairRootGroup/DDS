// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSPort__
#define __DDS__DDSPort__
// BOOST
#include <boost/property_tree/ptree.hpp>
// STL
#include <string>
#include <vector>

class DDSPort
{
  public:
    typedef std::pair<unsigned short, unsigned short> DDSPortRange_t;

    /**
     * \brief Constructor.
     */
    DDSPort();

    /**
     * \brief Destructor.
     */
    virtual ~DDSPort();

  public:
    /// Accessors
    void setName(const std::string& _name);
    void setRange(unsigned short _min, unsigned short _max);

    /// Modifiers
    std::string getName() const;
    const DDSPortRange_t& getRange() const;

    /**
     * \brief Initialize object with data from property tree.
     * \param[in] _name Name of the object as in input file.
     * \param[in] _pt Property tree starting from root.
     */
    void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const;

    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of
     * insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm, const DDSPort& _port);

  private:
    std::string m_name;     ///> Name of port.
    DDSPortRange_t m_range; ///> Requested range of ports.
};

typedef std::shared_ptr<DDSPort> DDSPortPtr_t;
typedef std::vector<DDSPortPtr_t> DDSPortPtrVector_t;

#endif /* defined(__DDS__DDSPort__) */
