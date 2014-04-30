// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__Port__
#define __DDS__Port__
// DDS
#include "TopoProperty.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
// STL
#include <string>
#include <vector>

namespace dds
{
    enum class EPortType
    {
        SERVER,
        CLIENT
    };

    class CPort : public CTopoProperty
    {
      public:
        typedef std::pair<unsigned short, unsigned short> PortRange_t;

        /// \brief Constructor.
        CPort();

        /// \brief Destructor.
        virtual ~CPort();

      public:
        /// Accessors
        void setRange(unsigned short _min, unsigned short _max);
        void setPortType(EPortType _portType);

        /// Modifiers
        const PortRange_t& getRange() const;
        EPortType getPortType() const;

        /// \brief Inherited from TopoBase
        void initFromPropertyTree(const std::string& _name, const boost::property_tree::ptree& _pt);

        /// \brief Returns string representation of an object.
        /// \return String representation of an object.
        virtual std::string toString() const;

        /// \brief Operator << for convenient output to ostream.
        /// \return Insertion stream in order to be able to call a succession of
        /// insertion operations.
        friend std::ostream& operator<<(std::ostream& _strm, const CPort& _port);

      private:
        PortRange_t m_range;  ///> Requested range of ports.
        EPortType m_portType; ///> Port type: server or client
    };

    typedef std::shared_ptr<CPort> PortPtr_t;
    typedef std::vector<PortPtr_t> PortPtrVector_t;
}
#endif /* defined(__DDS__Port__) */
