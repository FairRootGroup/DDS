// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_OCTOPUS_PROTOCOL_H
#define DDS_OCTOPUS_PROTOCOL_H

// DDS
#include "Intercom.h"
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
                m_sVal = _pt.data();
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
        // Get Ping command
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
        // Return command
        // {
        //   "ping": "text"
        // }
        struct SOctopusProtocol_Return : SOctopusProtocol_KeyValueImpl<SOctopusProtocol_Return>
        {
            static std::string class_name()
            {
                return "return";
            }
        };
        // Return command
        // {
        //   "big_cmd": "text"
        // }
        struct SOctopusProtocol_BigCmd : SOctopusProtocol_KeyValueImpl<SOctopusProtocol_BigCmd>
        {
            static std::string class_name()
            {
                return "big_cmd";
            }
        };
    } // namespace dds_octopus
} // namespace dds
#endif
