// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopology__
#define __DDS__DDSTopology__

// DDS
#include "DDSTaskGroup.h"
// STD
#include <sstream>
#include <string>

class DDSTopology
{
public:
    /**
     * \brief Constructor.
     */
    DDSTopology();

    /**
     * \brief Destructor.
     */
    virtual ~DDSTopology();

    /// Accessors
    DDSTaskGroupPtr_t getMainGroup() const
    {
        return m_main;
    }

    /**
     * \brief Returns string representation of an object.
     * \return String representation of an object.
     */
    std::string toString() const
    {
        std::stringstream ss;

        return ss.str();
    }

    /**
     * \brief Operator << for convenient output to ostream.
     * \return Insertion stream in order to be able to call a succession of
     * insertion operations.
     */
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTopology& _topology)
    {
        _strm << _topology.toString();
        return _strm;
    }

private:
    DDSTaskGroupPtr_t m_main; ///> Main task group which we run
};

#endif /* defined(__DDS__DDSTopology__) */
