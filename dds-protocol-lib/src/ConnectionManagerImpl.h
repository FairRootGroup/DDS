// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ConnectionManagerImpl__
#define __DDS__ConnectionManagerImpl__
// DDS
#include "CommandAttachmentImpl.h"
#include "MonitoringThread.h"
#include "Options.h"
#include "ProtocolMessage.h"
#include "StatImpl.h"
// STD
#include <mutex>
// BOOST
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include <boost/asio.hpp>
#pragma clang diagnostic pop
#include <boost/thread/thread.hpp>
// MiscCommon
#include "INet.h"

namespace dds
{
    namespace protocol_api
    {
        /// \class CConnectionManagerImpl
        /// \brief Base class for connection managers.
        template <class T, class A>
        class CConnectionManagerImpl
        {
          public:
            CConnectionManagerImpl(size_t _minPort, size_t _maxPort, bool _useUITransport)
            {
                int nSrvPort =
                    (_minPort == 0 && _maxPort == 0) ? 0 : MiscCommon::INet::get_free_port(_minPort, _maxPort);
                m_acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(
                    m_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), nSrvPort));

                if (_useUITransport)
                {
                    int nSrvPort =
                        (_minPort == 0 && _maxPort == 0) ? 0 : MiscCommon::INet::get_free_port(_minPort, _maxPort);
                    m_acceptorUI = std::make_shared<boost::asio::ip::tcp::acceptor>(
                        m_io_service_UI, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), nSrvPort));
                }

                // Create and register signals
                m_signals = std::make_shared<boost::asio::signal_set>(m_io_service);

                // Register to handle the signals that indicate when the server should exit.
                // It is safe to register for the same signal multiple times in a program,
                // provided all registration for the specified signal is made through Asio.
                m_signals->add(SIGINT);
                m_signals->add(SIGTERM);
#if defined(SIGQUIT)
                m_signals->add(SIGQUIT);
#endif // defined(SIGQUIT)

                m_signals->async_wait([this](boost::system::error_code /*ec*/, int signo) {
                    // The server is stopped by cancelling all outstanding asynchronous
                    // operations. Once all operations have finished the io_service::run()
                    // call will exit.
                    LOG(MiscCommon::info) << "Received a signal: " << signo;
                    LOG(MiscCommon::info) << "Sopping DDS transport server";

                    stop();
                });
            }

            ~CConnectionManagerImpl()
            {
                // Delete server info file
                deleteInfoFile();
                stop();
            }

            void start(bool _join = true, unsigned int _nThreads = 0 /*0 - auto; min. number is 4*/)
            {
                try
                {
                    // Call _start of the "child"
                    A* pThis = static_cast<A*>(this);
                    pThis->_start();

                    // Start monitoring thread
                    const float maxIdleTime =
                        user_defaults_api::CUserDefaults::instance().getOptions().m_server.m_idleTime;

                    CMonitoringThread::instance().start(maxIdleTime,
                                                        []() { LOG(MiscCommon::info) << "Idle callback called."; });
                    m_acceptor->listen();
                    createClientAndStartAccept(m_acceptor);

                    // If we use second channel for communication with UI we have to start acceptiing connection on that
                    // channel.
                    if (m_acceptorUI != nullptr)
                    {
                        m_acceptorUI->listen();
                        createClientAndStartAccept(m_acceptorUI);
                    }

                    // Create a server info file
                    createInfoFile();

                    // a thread pool for the DDS transport engine
                    // may return 0 when not able to detect
                    unsigned int concurrentThreads = (0 == _nThreads) ? std::thread::hardware_concurrency() : _nThreads;
                    // we need at least 2 threads
                    if (concurrentThreads < 2)
                        concurrentThreads = 2;
                    LOG(MiscCommon::info)
                        << "Starting DDS transport engine using " << concurrentThreads << " concurrent threads.";
                    for (int x = 0; x < concurrentThreads; ++x)
                    {
                        m_workerThreads.create_thread([this]() { runService(10, m_acceptor->get_io_service()); });
                    }

                    // Starting service for UI transport engine
                    if (m_acceptorUI != nullptr)
                    {
                        const unsigned int concurrentThreads = 2;
                        LOG(MiscCommon::info)
                            << "Starting DDS UI transport engine using " << concurrentThreads << " concurrent threads.";
                        for (int x = 0; x < concurrentThreads; ++x)
                        {
                            m_workerThreads.create_thread([this]() { runService(10, m_acceptorUI->get_io_service()); });
                        }
                    }

                    if (_join)
                        m_workerThreads.join_all();
                }
                catch (std::exception& e)
                {
                    LOG(MiscCommon::fatal) << e.what();
                }
            }

            void runService(short _counter, boost::asio::io_service& _io_service)
            {
                if (_counter <= 0)
                {
                    LOG(MiscCommon::error) << "CConnectionManagerImpl: can't start another io_service.";
                }
                try
                {
                    _io_service.run();
                }
                catch (std::exception& ex)
                {
                    LOG(MiscCommon::error) << "CConnectionManagerImpl exception: " << ex.what();
                    LOG(MiscCommon::info) << "CConnectionManagerImpl restarting io_service";
                    runService(--_counter, _io_service);
                }
            }

            void stop()
            {
                try
                {
                    // Call _stop of the "child"
                    A* pThis = static_cast<A*>(this);
                    pThis->_stop();

                    // Send shutdown signal to all client connections.
                    typename T::weakConnectionPtrVector_t channels(getChannels());

                    for (const auto& v : channels)
                    {
                        if (v.expired())
                            continue;
                        auto ptr = v.lock();
                        ptr->template pushMsg<cmdSHUTDOWN>();
                    }

                    auto condition = [](typename T::connectionPtr_t _v, bool& /*_stop*/) { return (_v->started()); };

                    size_t counter = 0;
                    while (true)
                    {
                        ++counter;
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        if (countNofChannels(condition) == 0)
                            break;
                        if (counter > 300)
                        {
                            LOG(MiscCommon::warning) << "Some channels were not shut down properly. Exiting in anyway.";
                            break;
                        }
                    }

                    m_acceptor->close();
                    m_acceptor->get_io_service().stop();

                    if (m_acceptor != nullptr)
                    {
                        m_acceptorUI->close();
                        m_acceptorUI->get_io_service().stop();
                    }

                    for (const auto& v : channels)
                    {
                        if (v.expired())
                            continue;
                        auto ptr = v.lock();
                        ptr->stop();
                    }

                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_channels.clear();
                }
                catch (std::bad_weak_ptr& e)
                {
                    // TODO: Do we need to log something here?
                }
                catch (std::exception& e)
                {
                    LOG(MiscCommon::fatal) << e.what();
                }
            }

          protected:
            typename T::weakConnectionPtrVector_t getChannels(
                std::function<bool(typename T::connectionPtr_t, bool&)> _condition = nullptr)
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                typename T::weakConnectionPtrVector_t result;
                result.reserve(m_channels.size());
                for (auto& v : m_channels)
                {
                    bool stop = false;
                    if (_condition == nullptr || _condition(v, stop))
                    {
                        result.push_back(v);
                        if (stop)
                            break;
                    }
                }
                return result;
            }

            template <ECmdType _cmd, class AttachmentType>
            void broadcastMsg(const AttachmentType& _attachment,
                              std::function<bool(typename T::connectionPtr_t, bool&)> _condition = nullptr)
            {
                try
                {
                    typename T::weakConnectionPtrVector_t channels(getChannels(_condition));

                    for (const auto& v : channels)
                    {
                        if (v.expired())
                            continue;
                        auto ptr = v.lock();
                        ptr->template pushMsg<_cmd>(_attachment);
                    }
                }
                catch (std::bad_weak_ptr& e)
                {
                    // TODO: Do we need to log something here?
                }
            }

            template <ECmdType _cmd, class AttachmentType>
            void accumulativeBroadcastMsg(const AttachmentType& _attachment,
                                          std::function<bool(typename T::connectionPtr_t, bool&)> _condition = nullptr)
            {
                try
                {
                    typename T::weakConnectionPtrVector_t channels(getChannels(_condition));

                    for (const auto& v : channels)
                    {
                        if (v.expired())
                            continue;
                        auto ptr = v.lock();
                        ptr->template accumulativePushMsg<_cmd>(_attachment);
                    }
                }
                catch (std::bad_weak_ptr& e)
                {
                    // TODO: Do we need to log something here?
                }
            }

            template <ECmdType _cmd>
            void broadcastSimpleMsg(std::function<bool(typename T::connectionPtr_t, bool&)> _condition = nullptr)
            {
                SEmptyCmd cmd;
                broadcastMsg<_cmd>(cmd, _condition);
            }

            void broadcastBinaryAttachmentCmd(
                const MiscCommon::BYTEVector_t& _data,
                const std::string& _fileName,
                uint16_t _cmdSource,
                std::function<bool(typename T::connectionPtr_t, bool&)> _condition = nullptr)
            {
                try
                {
                    typename T::weakConnectionPtrVector_t channels(getChannels(_condition));

                    for (const auto& v : channels)
                    {
                        if (v.expired())
                            continue;
                        auto ptr = v.lock();
                        ptr->pushBinaryAttachmentCmd(_data, _fileName, _cmdSource);
                    }
                }
                catch (std::bad_weak_ptr& e)
                {
                    // TODO: Do we need to log something here?
                }
            }

            size_t countNofChannels(std::function<bool(typename T::connectionPtr_t, bool&)> _condition = nullptr)
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                if (_condition == nullptr)
                    return m_channels.size();
                size_t counter = 0;
                for (auto& v : m_channels)
                {
                    bool stop = false;
                    if (_condition(v, stop))
                    {
                        counter++;
                        if (stop)
                            break;
                    }
                }
                return counter;
            }

          private:
            void acceptHandler(typename T::connectionPtr_t _client,
                               std::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor,
                               const boost::system::error_code& _ec)
            {
                if (!_ec)
                {
                    _client->start();
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        m_channels.push_back(_client);
                    }
                    createClientAndStartAccept(_acceptor);
                }
                else
                {
                    LOG(MiscCommon::error) << "Can't accept new connection: " << _ec.message();
                }
            }

            void createClientAndStartAccept(std::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor)
            {
                typename T::connectionPtr_t newClient = T::makeNew(_acceptor->get_io_service(), 0);

                A* pThis = static_cast<A*>(this);
                pThis->newClientCreated(newClient);

                // Subscribe on dissconnect event
                newClient->template registerHandler<EChannelEvents::OnRemoteEndDissconnected>(
                    [this, newClient]() -> void {
                        {
                            // collect statistics for disconnected channels
                            std::lock_guard<std::mutex> lock(m_statMutex);
                            m_readStatDisconnectedChannels.addFromStat(newClient->getReadStat());
                            m_writeStatDisconnectedChannels.addFromStat(newClient->getWriteStat());
                        }
                        this->removeClient(newClient.get());
                    });

                _acceptor->async_accept(
                    newClient->socket(),
                    std::bind(
                        &CConnectionManagerImpl::acceptHandler, this, newClient, _acceptor, std::placeholders::_1));
            }

            void createInfoFile()
            {
                // The child needs to have that method
                A* pThis = static_cast<A*>(this);

                std::vector<size_t> ports;
                ports.push_back(m_acceptor->local_endpoint().port());
                if (m_acceptorUI != nullptr)
                    ports.push_back(m_acceptorUI->local_endpoint().port());

                pThis->_createInfoFile(ports);
            }

            void deleteInfoFile()
            {
                // The child needs to have that method
                A* pThis = static_cast<A*>(this);
                pThis->_deleteInfoFile();
            }

            void removeClient(T* _client)
            {
                // TODO: fix getTypeName call
                LOG(MiscCommon::debug) << "Removing " /*<< _client->getTypeName()*/
                                       << " client from the list of active";
                std::lock_guard<std::mutex> lock(m_mutex);
                m_channels.erase(remove_if(m_channels.begin(),
                                           m_channels.end(),
                                           [&](typename T::connectionPtr_t& i) { return (i.get() == _client); }),
                                 m_channels.end());
            }

          public:
            void addDisconnectedChannelsStatToStat(SReadStat& _readStat, SWriteStat& _writeStat)
            {
                // Add disconnected channels statistics to some external statistics.
                // This is done in order not to copy self stat structures and return them.
                // Or not to return reference to self stat together with mutex.
                std::lock_guard<std::mutex> lock(m_statMutex);
                _readStat.addFromStat(m_readStatDisconnectedChannels);
                _writeStat.addFromStat(m_writeStatDisconnectedChannels);
            }

          private:
            /// The signal_set is used to register for process termination notifications.
            std::shared_ptr<boost::asio::signal_set> m_signals;
            std::mutex m_mutex;
            typename T::connectionPtrVector_t m_channels;

            /// Used for the main comunication
            boost::asio::io_service m_io_service;
            std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;

            // Used for UI (priority) communication
            boost::asio::io_service m_io_service_UI;
            std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptorUI;

            boost::thread_group m_workerThreads;

            // Statistics of disconnected channels
            SReadStat m_readStatDisconnectedChannels;
            SWriteStat m_writeStatDisconnectedChannels;
            std::mutex m_statMutex;
        };
    }
}
#endif /* defined(__DDS__ConnectionManagerImpl__) */
