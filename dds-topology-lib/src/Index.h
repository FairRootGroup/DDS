// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__Index__
#define __DDS__Index__

#include <string>
#include <vector>

namespace dds
{
    class CIndex
    {
      public:
        /// \brief Constructor
        CIndex();

        /// \brief Constructor with path.
        /// \param _path Path to element in topology.
        CIndex(const std::string& _path);

        /// \ breif Destructor
        ~CIndex();

        /// Accessors
        std::string getPath() const;

        /// \brief Returns string representation of an object.
        /// \return String representation of an object.
        virtual std::string toString() const;

        /// \brief Operator << for convenient output to ostream.
        /// \return Insertion stream in order to be able to call a succession of
        /// insertion operations.
        friend std::ostream& operator<<(std::ostream& _strm, const CIndex& _index);

      private:
        std::string m_path; ///> Path to element in topology
    };

    class CompareIndexLess : public std::binary_function<const CIndex&, const CIndex&, bool>
    {
      public:
        bool operator()(const CIndex& index1, const CIndex& index2) const;
    };

    typedef std::vector<CIndex> IndexVector_t;
}

#endif /* defined(__DDS__Index__) */
