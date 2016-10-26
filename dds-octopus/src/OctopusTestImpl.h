// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_OCTOPUS_TESTIMPL_H
#define DDS_OCTOPUS_TESTIMPL_H

// DDS
#include "OctopusProtocol.h"
#include "Options.h"
#include "dds_intercom.h"
// STD
#include <iostream>
// BOOST
#include <boost/property_tree/json_parser.hpp>

#define BEGIN_OCTOPUS_MSG_MAP(name)                                                                   \
    std::string _name() const                                                                         \
    {                                                                                                 \
        return name;                                                                                  \
    }                                                                                                 \
                                                                                                      \
    void _onCustomCmd(const std::string& _command, const std::string& _condition, uint64_t _senderId) \
    {                                                                                                 \
        boost::property_tree::ptree root;                                                             \
        std::stringstream ss;                                                                         \
        ss << _command;                                                                               \
        boost::property_tree::read_json(ss, root);                                                    \
        for (const auto& node : root)                                                                 \
        {                                                                                             \
            if (node.first == dds_octopus::SOctopusProtocol_Log::class_name())                        \
            {                                                                                         \
                dds_octopus::SOctopusProtocol_Log log;                                                \
                log.init(node.second);                                                                \
                std::cout << "Task: " << log.m_sVal << std::endl;                                     \
                continue;                                                                             \
            }

#define OCTOPUS_MSG_HANDLER(func, cmd)        \
    else if (node.first == cmd::class_name()) \
    {                                         \
        cmd var;                              \
        var.init(node.second);                \
        func(var, _senderId);                 \
        continue;                             \
    }

#define END_OCTOPUS_MSG_MAP \
    }                       \
    }

namespace dds
{
    namespace dds_octopus
    {
        enum ETestStatus
        {
            TS_OK,
            TS_FAILED,
            TS_TIMEOUT
        };

        template <class T>
        class COctopusTestImpl
        {
          public:
            COctopusTestImpl(const SOptions_t& _options)
                : m_customCmd(m_intercomService)
                , m_options(_options)
            {
            }
            ~COctopusTestImpl()
            {
            }

          public:
            void init()
            {
                m_intercomService.subscribeOnError(boost::bind(&COctopusTestImpl::onServiceError, this, _1, _2));
                m_customCmd.subscribe(boost::bind(&COctopusTestImpl::onCustomCmd, this, _1, _2, _3));

                m_intercomService.start();
            }

            ETestStatus execute(const size_t _timeoutInSec)
            {
                auto start_time = std::chrono::high_resolution_clock::now();

                T* pThis = static_cast<T*>(this);
                std::cout << "================================\n";
                std::cout << "Test case: " << pThis->_name() << "\n";

                pThis->_init();

                std::unique_lock<std::mutex> lock(m_waitMutex);
                if (std::cv_status::timeout ==
                    m_waitCondition.wait_until(lock, start_time + std::chrono::seconds(_timeoutInSec)))
                {
                    m_status = TS_TIMEOUT;
                    std::cout << "RESULT: FAILED. Timeout (" << _timeoutInSec << " sec) reached\n";
                }
                else
                {
                    switch (m_status)
                    {
                        case TS_OK:
                            std::cout << "RESULT: OK\n";
                            break;
                        default:
                            std::cout << "RESULT: FAILED\n";
                    }
                }
                auto execTime = std::chrono::high_resolution_clock::now() - start_time;
                std::chrono::milliseconds execTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(execTime);
                std::cout << "Execution time: " << std::chrono::duration<double>(execTimeMs).count() << " s\n";
                std::cout << "================================" << std::endl;

                return m_status;
            }

          protected:
            void onServiceError(const dds::intercom_api::EErrorCode _errorCode, const std::string& _errorMsg)
            {
                std::cerr << "Error: " << _errorMsg << std::endl;
            }

            void onCustomCmd(const std::string& _command, const std::string& _condition, uint64_t _senderId)
            {
                T* pThis = static_cast<T*>(this);
                pThis->_onCustomCmd(_command, _condition, _senderId);
            }

          protected:
            dds::intercom_api::CIntercomService m_intercomService;
            dds::intercom_api::CCustomCmd m_customCmd;
            std::mutex m_waitMutex;
            std::condition_variable m_waitCondition;
            ETestStatus m_status;
            SOptions_t m_options;
        };
    }
}
#endif
