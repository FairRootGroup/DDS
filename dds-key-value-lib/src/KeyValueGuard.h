// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_KeyValueGuard_h
#define DDS_KeyValueGuard_h
// DDS
#include "AgentChannel.h"
// STD
#include <string>
// BOOST
#include <boost/property_tree/ptree.hpp>

namespace dds
{

    class CKeyValueGuard
    {
      private:
        CKeyValueGuard();
        ~CKeyValueGuard();

      public:
        static CKeyValueGuard& instance();
        void putValue(const std::string& _key, const std::string& _value, const std::string& _taskId);
        void getValue(const std::string& _key, std::string* _value, const std::string& _taskId);
        void notifyAgent(const SCommandContainer& _newCommand);

      private:
        void init();
        const std::string getCfgFilePath() const;

      private:
        boost::property_tree::ptree m_pt;
    };
}

#endif
