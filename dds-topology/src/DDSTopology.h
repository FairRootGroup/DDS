// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSTopology__
#define __DDS__DDSTopology__

// DDS
#include "DDSIndex.h"
#include "DDSTaskGroup.h"
#include "DDSTopoElement.h"
// STD
#include <ostream>
#include <string>
#include <map>

class DDSIndex;

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
    DDSTopoElementPtr_t getTopoElementByIndex(const DDSIndex& _index) const;

    /// \brief Returns string representation of an object.
    /// \return String representation of an object.
    virtual std::string toString() const;

    /// \brief Operator << for convenient output to ostream.
    /// \return Insertion stream in order to be able to call a succession of
    /// insertion operations.
    friend std::ostream& operator<<(std::ostream& _strm, const DDSTopology& _topology);

  private:
    void FillIndexToTopoElementMap(const DDSTopoElementPtr_t& _element);

    DDSTaskGroupPtr_t m_main; ///> Main task group which we run

    typedef std::map<DDSIndex, DDSTopoElementPtr_t, DDSCompareIndexLess> DDSIndexToTopoElementMap_t;
    DDSIndexToTopoElementMap_t m_indexToTopoElementMap;
};

#endif /* defined(__DDS__DDSTopology__) */
