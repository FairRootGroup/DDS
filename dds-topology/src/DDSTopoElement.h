// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopoElement__
#define __DDS__DDSTopoElement__

// STD
#include <sstream>
#include <string>
#include <vector>

enum class DDSTopoElementType
{
    NONE,
    TASK,
    COLLECTION,
    GROUP
};

class DDSTopoElement
{
public:
    /// Modifiers
    void setName(const std::string& _name)
    {
        m_name = _name;
    }

    /// Accessors
    std::string getName() const
    {
        return m_name;
    }

    DDSTopoElementType getType() const
    {
        return m_type;
    }

    /**
     * \brief Has to return number of all tasks including daughter elements.
     */
    virtual size_t getNofTasks() const = 0;

    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const
    {
        std::stringstream ss;
        ss << "DDSTopoElement: m_name=" << m_name << std::endl;
        return ss.str();
    }

    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of
     * insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTopoElement& _element)
    {
        _strm << _element.toString();
        return _strm;
    }

protected:
    /**
     * \brief Constructor.
     */
    DDSTopoElement()
        : m_name("")
        , m_type(DDSTopoElementType::NONE)
    {
    }

    /**
     * \brief Destructor.
     */
    virtual ~DDSTopoElement()
    {
    }

    void setType(DDSTopoElementType _type)
    {
        m_type = _type;
    }

private:
    std::string m_name;        ///> Name of topology element
    DDSTopoElementType m_type; ///> Type of the topology element

    // FIXME: Probably we have to add an ID which will uniquely identifies the object.
};

typedef std::shared_ptr<DDSTopoElement> DDSTopoElementPtr_t;
typedef std::vector<DDSTopoElementPtr_t> DDSTopoElementPtrVector_t;

#endif /* defined(__DDS__DDSTopoElement__) */
