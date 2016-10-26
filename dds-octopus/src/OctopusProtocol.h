// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_OCTOPUS_PROTOCOL_H
#define DDS_OCTOPUS_PROTOCOL_H

// DDS
#include "dds_intercom.h"
// BOOST
#include <boost/property_tree/ptree.hpp>

namespace dds
{
    namespace dds_octopus
    {
        template <class T>
        struct SOctopusProtocol_KeyValueImpl
        {
            void init(const boost::property_tree::ptree& _pt)
            {
                T* pThis = static_cast<T*>(this);
                m_sVal = _pt.get<std::string>(pThis->class_name(), "");
            }

            void get(boost::property_tree::ptree* _pt)
            {
                if (!_pt)
                    return;

                T* pThis = static_cast<T*>(this);
                _pt->put(pThis->class_name(), m_sVal);
            }

            std::string m_sVal;
        };

        // Log command
        // {
        //   "log": "text"
        // }
        struct SOctopusProtocol_Log : SOctopusProtocol_KeyValueImpl<SOctopusProtocol_Log>
        {
            static std::string class_name()
            {
                return "log";
            }
        };
        // Log command
        // {
        //   "get_ping": "text"
        // }
        struct SOctopusProtocol_GetPing : SOctopusProtocol_KeyValueImpl<SOctopusProtocol_GetPing>
        {
            static std::string class_name()
            {
                return "get_ping";
            }
        };
        // Log command
        // {
        //   "ping": "text"
        // }
        struct SOctopusProtocol_Ping : SOctopusProtocol_KeyValueImpl<SOctopusProtocol_Ping>
        {
            static std::string class_name()
            {
                return "ping";
            }
        };
    }
}
#endif
