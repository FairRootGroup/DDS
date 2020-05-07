// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoVars__
#define __DDS__TopoVars__

// DDS
#include "TopoBase.h"
// STD
#include <map>
#include <string>

namespace dds
{
    namespace topology_api
    {
        class CTopoVars : public CTopoBase
        {
          public:
            using Ptr_t = std::shared_ptr<CTopoVars>;
            using varMap_t = std::map<std::string, std::string>;

            /// \brief Constructor.
            CTopoVars(const std::string& _name);

            /// \brief Destructor.
            virtual ~CTopoVars();

            /// \brief Inherited from TopoBase
            void initFromPropertyTree(const boost::property_tree::ptree& _pt);

            /// \brief Inherited from TopoBase
            void saveToPropertyTree(boost::property_tree::ptree& _pt);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoVars& _vars);

            /// \brief Inherited from TopoBase
            virtual std::string hashString() const;

            const varMap_t& getMap() const;

          private:
            varMap_t m_map; ///< Key-Value storage of variables
        };
    } // namespace topology_api
} // namespace dds

#endif /* defined(__DDS__TopoVars__) */
