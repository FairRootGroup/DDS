// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSPort__
#define __DDS__DDSPort__

#include <sstream>
#include <string>
#include <vector>
#include <memory>

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
    void setName(const std::string& _name);
    void setRange(unsigned short _min, unsigned short _max);
    std::string getName() const;
    const DDSPortRange_t& getRange() const;
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
    friend std::ostream& operator<<(std::ostream& _strm, const DDSPort& _port)
    {
        _strm << _port.toString();
        return _strm;
    }

  private:
    std::string m_name;     ///> Name of port.
    DDSPortRange_t m_range; ///> Requested range of ports.
};

typedef std::shared_ptr<DDSPort> DDSPortPtr_t;
// typedef std::vector<DDSPortPtr_t> DDSPortPtrVector_t;
typedef std::vector<DDSPort> DDSPortVector_t;

#endif /* defined(__DDS__DDSPort__) */
