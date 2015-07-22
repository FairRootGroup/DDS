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
#include <vector>
// BOOST
#include <boost/signals2/signal.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <boost/interprocess/sync/named_mutex.hpp>
#pragma clang diagnostic pop
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>

namespace dds
{
    typedef boost::signals2::signal<void(const std::string&, const std::string&)> signal_t;
    typedef boost::signals2::signal<void(const std::string&)> errorSignal_t;
    typedef boost::signals2::connection connection_t;
    typedef std::shared_ptr<boost::interprocess::named_mutex> sharedMemoryMutexPtr_t;
    typedef std::shared_ptr<boost::interprocess::managed_shared_memory> managedSharedMemoryPtr_t;
    typedef boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager>
        sharedMemoryCharAllocator_t;
    typedef boost::interprocess::basic_string<char, std::char_traits<char>, sharedMemoryCharAllocator_t>
        sharedMemoryString_t;
    typedef boost::interprocess::basic_vectorstream<sharedMemoryString_t> sharedMemoryVectorStream_t;

    struct SSyncHelper
    {
        signal_t m_updateSig;
        errorSignal_t m_errorSig;
    };

    class CKeyValueGuard
    {
        typedef std::shared_ptr<agent::CAgentConnectionManager> AgentConnectionManagerPtr_t;
        typedef std::map<std::string, std::string> valuesMap_t;
        typedef std::vector<std::pair<std::string, std::string>> valuesVector_t;

      private:
        CKeyValueGuard();
        ~CKeyValueGuard();

      public:
        static CKeyValueGuard& instance();
        void createStorage();
        void initLock();
        void putValue(const std::string& _key, const std::string& _value, const std::string& _taskId);
        void putValue(const std::string& _key, const std::string& _value);
        void putValues(const std::vector<SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t>& _values);
        void getValue(const std::string& _key, std::string* _value, const std::string& _taskId);
        void getValues(const std::string& _key, valuesMap_t* _values);
        int updateKey(const SUpdateKeyCmd& _cmd);
        void deleteKey(const std::string& _key);
        connection_t connect(signal_t::slot_function_type _subscriber)
        {
            return m_syncHelper.m_updateSig.connect(_subscriber);
        }
        void disconnect()
        {
            return m_syncHelper.m_updateSig.disconnect_all_slots();
        }
        connection_t connectError(errorSignal_t::slot_function_type _subscriber)
        {
            return m_syncHelper.m_errorSig.connect(_subscriber);
        }
        void disconnectError()
        {
            return m_syncHelper.m_errorSig.disconnect_all_slots();
        }
        static void clean();

        // User API
        void initAgentConnection();

      public:
        SSyncHelper m_syncHelper;

      private:
        const std::string getCfgFilePath() const;

      private:
        AgentConnectionManagerPtr_t m_agentConnectionMng;
        std::string m_sCfgFilePath;
        sharedMemoryMutexPtr_t m_sharedMemoryMutex;
        managedSharedMemoryPtr_t m_sharedMemory;

        std::mutex m_initAgentConnectionMutex;
    };
}

#endif
