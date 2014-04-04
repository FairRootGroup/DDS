// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__DDSIndex__
#define __DDS__DDSIndex__

#include <string>
#include <vector>

class DDSIndex
{
  public:
    /// \brief Constructor
    DDSIndex();

    /// \brief Constructor with path.
    /// \param _path Path to element in topology.
    DDSIndex(const std::string& _path);

    /// \ breif Destructor
    ~DDSIndex();

    /// Accessors
    std::string getPath() const;

    /// \brief Returns string representation of an object.
    /// \return String representation of an object.
    virtual std::string toString() const;

    /// \brief Operator << for convenient output to ostream.
    /// \return Insertion stream in order to be able to call a succession of
    /// insertion operations.
    friend std::ostream& operator<<(std::ostream& _strm, const DDSIndex& _index);

  private:
    std::string m_path; ///> Path to element in topology
};

class DDSCompareIndexLess : public std::binary_function<const DDSIndex&, const DDSIndex&, bool>
{
  public:
    bool operator()(const DDSIndex& index1, const DDSIndex& index2) const;
};

typedef std::vector<DDSIndex> DDSIndexVector_t;

#endif /* defined(__DDS__DDSIndex__) */
