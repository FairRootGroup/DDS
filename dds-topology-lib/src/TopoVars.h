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
// BOOST
#include <boost/filesystem.hpp>

namespace dds
{
    namespace topology_api
    {
        class CTopoVars : public CTopoBase
        {
          public:
            using Ptr_t = std::shared_ptr<CTopoVars>;
            using varName_t = std::string;
            using varValue_t = std::string;
            using varMap_t = std::map<varName_t, varValue_t>;
            using propTreePtr_t = std::unique_ptr<boost::property_tree::ptree>;

            /// \brief Constructor.
            CTopoVars();
            CTopoVars(const std::string& _name);

            /// \brief Destructor.
            virtual ~CTopoVars();

            /// \brief Init the object from DDS topology xml
            void initFromXML(const std::string& _filepath);

            /// \brief Serizalize the object to DDS topology xml
            void saveToXML(const std::string& _filepath);

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

            /// \brief Add a new topology variable.
            /// \return a bool denoting whether the insertion took place.
            bool add(const varName_t& _name, const varValue_t& _value);
            /// \brief Update existing topology variable.
            /// \return a bool denoting whether the update took place.
            bool update(const varName_t& _name, const varValue_t& _newValue);

            const varMap_t& getMap() const;

          private:
            varMap_t m_map; ///< Key-Value storage of variables
            propTreePtr_t m_pPropTreePtr;
        };
    } // namespace topology_api
} // namespace dds

#endif /* defined(__DDS__TopoVars__) */
