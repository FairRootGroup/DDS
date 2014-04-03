// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopology__
#define __DDS__DDSTopology__

// DDS
#include "DDSTaskGroup.h"
#include "DDSTopoElement.h"
// STD
#include <ostream>
#include <string>
#include <map>

class DDSTopology
{
  public:
    /// \brief Constructor.
    DDSTopology();

    /// \brief Destructor.
    virtual ~DDSTopology();

    /// \brief Initializes topology from specified file.
    /// \throw runtime_error
    void init(const std::string& _fileName);

    /// Accessors
    DDSTaskGroupPtr_t getMainGroup() const;
    DDSTopoElementPtr_t getTopoElementByPath(const std::string& _path) const;

    /// \brief Returns string representation of an object.
    /// \return String representation of an object.
    virtual std::string toString() const;

    /// \brief Operator << for convenient output to ostream.
    /// \return Insertion stream in order to be able to call a succession of
    /// insertion operations.
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTopology& _topology);

  private:
    void FillPathToTopoElementMap(const DDSTopoElementPtr_t& _element);

    DDSTaskGroupPtr_t m_main; ///> Main task group which we run

    typedef std::map<std::string, DDSTopoElementPtr_t> DDSPathToTopoElementMap_t;
    DDSPathToTopoElementMap_t m_pathToTopoElementMap;
};

#endif /* defined(__DDS__DDSTopology__) */
