// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDSINTERCOMGUARD_H
#define DDSINTERCOMGUARD_H
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
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>

namespace dds
{
    namespace internal_api
    {
        // Key-value types
        typedef boost::signals2::signal<void(const std::string&, const std::string&)> keyValueSignal_t;
        typedef boost::signals2::signal<void(const std::string&)> keyValueErrorSignal_t;
        typedef std::shared_ptr<boost::interprocess::named_mutex> sharedMemoryMutexPtr_t;
        typedef std::shared_ptr<boost::interprocess::managed_shared_memory> managedSharedMemoryPtr_t;
        typedef boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager>
            sharedMemoryCharAllocator_t;
        typedef boost::interprocess::basic_string<char, std::char_traits<char>, sharedMemoryCharAllocator_t>
            sharedMemoryString_t;
        typedef boost::interprocess::basic_vectorstream<sharedMemoryString_t> sharedMemoryVectorStream_t;

        // Custom command types
        typedef boost::signals2::signal<void(const std::string&, const std::string&, uint64_t)> customCmdSignal_t;
        typedef boost::signals2::signal<void(const std::string&)> customCmdReplySignal_t;

        typedef boost::signals2::connection connection_t;

        struct SSyncHelper
        {
            keyValueSignal_t m_keyValueUpdateSig;
            keyValueErrorSignal_t m_keyValueErrorSig;
            customCmdSignal_t m_customCmdSignal;
            customCmdReplySignal_t m_customCmdReplySignal;
        };

        class CDDSIntercomGuard
        {
            typedef std::shared_ptr<CAgentConnectionManager> AgentConnectionManagerPtr_t;

            typedef std::map<std::string, std::string> valuesMap_t;
            typedef std::vector<std::pair<std::string, std::string>> valuesVector_t;

          private:
            CDDSIntercomGuard();
            ~CDDSIntercomGuard();

          public:
            static CDDSIntercomGuard& instance();

            connection_t connectCustomCmd(customCmdSignal_t::slot_function_type _subscriber);
            connection_t connectCustomCmdReply(customCmdReplySignal_t::slot_function_type _subscriber);
            void disconnectCustomCmd();
            void disconnectKeyValue();

            int sendCustomCmd(const protocol_api::SCustomCmdCmd& _command);

            void createStorage();
            void initLock();
            void putValue(const std::string& _key, const std::string& _value, const std::string& _taskId);
            void putValue(const std::string& _key, const std::string& _value);
            void putValues(
                const std::vector<protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t>& _values);
            void getValue(const std::string& _key, std::string* _value, const std::string& _taskId);
            void getValues(const std::string& _key, valuesMap_t* _values);
            int updateKey(const protocol_api::SUpdateKeyCmd& _cmd);
            void deleteKey(const std::string& _key);
            connection_t connectKeyValue(keyValueSignal_t::slot_function_type _subscriber);
            connection_t connectKeyValueError(keyValueErrorSignal_t::slot_function_type _subscriber);
            static void clean();

            void waitCondition();
            void stopCondition();

            void initAgentConnection();

          public:
            SSyncHelper m_syncHelper;

          private:
            AgentConnectionManagerPtr_t m_agentConnectionMng;
            std::mutex m_initAgentConnectionMutex;

            std::string m_sCfgFilePath;
            sharedMemoryMutexPtr_t m_sharedMemoryMutex;
            managedSharedMemoryPtr_t m_sharedMemory;
        };
    }
}

#endif
