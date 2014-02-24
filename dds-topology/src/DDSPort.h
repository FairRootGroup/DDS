//
//  DDSPort.h
//  DDS
//
//  Created by Andrey Lebedev on 2/24/14.
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
    DDSPort()
        : m_name("")
        , m_range(std::make_pair(10000, 50000))
    {
    }

    /**
     * \brief Destructor.
     */
    virtual ~DDSPort()
    {
    }

    void setName(const std::string& _name)
    {
        m_name = _name;
    }
    void setRange(unsigned short _min, unsigned short _max)
    {
        m_range = std::make_pair(_min, _max);
    }

    std::string getName() const
    {
        return m_name;
    }
    const DDSPortRange_t& getRange() const
    {
        return m_range;
    }

    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const
    {
        std::stringstream ss;
        ss << "DDSPort: m_name=" << m_name << " m_range=(" << m_range.first << ", " << m_range.second << ")";
        return ss.str();
    }

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
typedef std::vector<DDSPortPtr_t> DDSPortPtrVector_t;

#endif /* defined(__DDS__DDSPort__) */
