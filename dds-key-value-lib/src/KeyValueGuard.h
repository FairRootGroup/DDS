// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_KeyValueGuard_h
#define DDS_KeyValueGuard_h
// DDS
#include "AgentConnectionManager.h"
// STD
#include <string>
// BOOST
#include <boost/signals2/signal.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

namespace dds
{
    typedef boost::signals2::signal<void(const std::string&, const std::string&)> signal_t;
    typedef boost::signals2::connection connection_t;
    typedef std::shared_ptr<boost::interprocess::named_mutex> fileMutexPtr_t;

    struct SSyncHelper
    {
        // std::condition_variable m_cvUpdateKey;
        // std::mutex m_mtxUpdateKey;
        signal_t m_updateSig;
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
        void createStorage();
        void initLock();
        void putValue(const std::string& _key, const std::string& _value, const std::string& _taskId);
        void putValue(const std::string& _key, const std::string& _value);
        void getValue(const std::string& _key, std::string* _value, const std::string& _taskId);
        void getValues(const std::string& _key, valuesMap_t* _values);
        int updateKey(const SUpdateKeyCmd& _cmd);
        connection_t connect(signal_t::slot_function_type _subscriber)
        {
            return m_syncHelper.m_updateSig.connect(_subscriber);
        }

        // User API
        void initAgentConnection();

      public:
        SSyncHelper m_syncHelper;

      private:
        const std::string getCfgFilePath() const;

      private:
        AgentConnectionManagerPtr_t m_agentConnectionMng;
        std::string m_sCfgFilePath;
        fileMutexPtr_t m_fileMutex;
    };
}

#endif
