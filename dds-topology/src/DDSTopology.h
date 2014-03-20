// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopology__
#define __DDS__DDSTopology__

// DDS
#include "DDSTaskGroup.h"
// STD
#include <ostream>
#include <string>

class DDSTopology
{
  public:
    /// \brief Constructor.
    DDSTopology();

    /// \brief Destructor.
    virtual ~DDSTopology();

    /// Accessors
    DDSTaskGroupPtr_t getMainGroup() const;

    /// \brief Returns string representation of an object.
    /// \return String representation of an object.
    virtual std::string toString() const;

    /// \brief Operator << for convenient output to ostream.
    /// \return Insertion stream in order to be able to call a succession of
    /// insertion operations.
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTopology& _topology);

  private:
    DDSTaskGroupPtr_t m_main; ///> Main task group which we run
};

#endif /* defined(__DDS__DDSTopology__) */
