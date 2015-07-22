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
            typedef std::map<std::string, std::string> varMap_t;

            /// \brief Constructor.
            CTopoVars();

            /// \brief Destructor.
            virtual ~CTopoVars();

            /// \brief Inherited from TopoBase
            void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoVars& _vars);

            const varMap_t& getMap() const;

          private:
            varMap_t m_map; ///> Key-Value storage of variables
        };

        typedef std::shared_ptr<CTopoVars> TopoVarsPtr_t;
    }
}

#endif /* defined(__DDS__TopoVars__) */
