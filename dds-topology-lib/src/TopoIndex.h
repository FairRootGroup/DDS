// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoIndex__
#define __DDS__TopoIndex__

#include <string>
#include <vector>

namespace dds
{
    namespace topology_api
    {
        class CTopoIndex
        {
          public:
            /// \brief Constructor
            CTopoIndex();

            /// \brief Constructor with path.
            /// \param _path Path to element in topology.
            CTopoIndex(const std::string& _path);

            /// \ breif Destructor
            virtual ~CTopoIndex();

            /// Accessors
            std::string getPath() const;

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoIndex& _index);

          private:
            std::string m_path; ///< Path to element in topology
        };

        class CompareTopoIndexLess : public std::binary_function<const CTopoIndex&, const CTopoIndex&, bool>
        {
          public:
            bool operator()(const CTopoIndex& index1, const CTopoIndex& index2) const;
        };

        typedef std::vector<CTopoIndex> TopoIndexVector_t;
    } // namespace topology_api
} // namespace dds

#endif /* defined(__DDS__TopoIndex__) */
