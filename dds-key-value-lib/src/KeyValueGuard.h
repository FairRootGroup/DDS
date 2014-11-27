// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_KeyValueGuard_h
#define DDS_KeyValueGuard_h
// DDS
#include "AgentChannel.h"
#include "AgentConnectionManager.h"
// STD
#include <string>
// BOOST
#include <boost/property_tree/ptree.hpp>

namespace dds
{
    struct SSyncHelper
    {
        std::condition_variable m_cvUpdateKey;
        std::mutex m_mtxUpdateKey;
        std::condition_variable m_cvWaitKey;
        std::mutex m_mtxWaitKey;
    };

    class CKeyValueGuard
    {
        typedef std::shared_ptr<CAgentConnectionManager> AgentConnectionManagerPtr_t;
        typedef std::map<std::string, std::string> valuesMap_t;

      private:
        CKeyValueGuard();
        ~CKeyValueGuard();

      public:
        static CKeyValueGuard& instance();
        void putValue(const std::string& _key, const std::string& _value, const std::string& _taskId);
        void getValue(const std::string& _key, std::string* _value, const std::string& _taskId);
        void getValues(const std::string& _key, valuesMap_t* _values);
        int updateKey(const SUpdateKeyCmd& _cmd);

        // User API
        void initAgentConnection();

      public:
        SSyncHelper m_syncHelper;

      private:
        void init();
        const std::string getCfgFilePath() const;

      private:
        boost::property_tree::ptree m_pt;
        AgentConnectionManagerPtr_t m_agentConnectionMng;
        std::mutex m_mtxAgentConnnection;
    };
}

#endif
