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
        // Log command
        // {
        //   "log":
        //   {
        //      "msg": "text"
        //   }
        // }
        struct SOctopusProtocol_Log
        {
            enum
            {
                class_id = 1
            };
            static std::string class_name()
            {
                return "log";
            }
            void init(const boost::property_tree::ptree& _pt)
            {
                m_sMsg = _pt.get<std::string>("msg");
            }

            std::string m_sMsg;
        };
        // Log command
        // {
        //   "get_ping": "text"
        // }
        struct SOctopusProtocol_GetPing
        {
            enum
            {
                class_id = 1
            };
            static std::string class_name()
            {
                return "get_ping";
            }
            void init(const boost::property_tree::ptree& _pt)
            {
                m_sMsg = _pt.get<std::string>(class_name(), "");
            }

            void get(boost::property_tree::ptree* _pt)
            {
                if (!_pt)
                    return;

                _pt->put(class_name(), m_sMsg);
            }

            std::string m_sMsg;
        };
        // Log command
        // {
        //   "ping": "text"
        // }
        struct SOctopusProtocol_Ping
        {
            enum
            {
                class_id = 1
            };
            static std::string class_name()
            {
                return "ping";
            }
            void init(const boost::property_tree::ptree& _pt)
            {
                m_sMsg = _pt.get<std::string>(class_name(), "");
            }

            void get(boost::property_tree::ptree* _pt)
            {
                if (!_pt)
                    return;

                _pt->put(class_name(), m_sMsg);
            }

            std::string m_sMsg;
        };
    }
}
#endif
