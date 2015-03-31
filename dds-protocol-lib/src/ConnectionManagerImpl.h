// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ConnectionManagerImpl__
#define __DDS__ConnectionManagerImpl__
// DDS
#include "MonitoringThread.h"
#include "Options.h"
#include "ProtocolMessage.h"
#include "CommandAttachmentImpl.h"
// STD
#include <mutex>
// BOOST
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

namespace dds
{
    template <class T, class A>
    class CConnectionManagerImpl
    {
      public:
        CConnectionManagerImpl(boost::asio::io_service& _io_service, boost::asio::ip::tcp::endpoint& _endpoint)
            : m_acceptor(_io_service, _endpoint)
            , m_signals(_io_service)
        {
            // Register to handle the signals that indicate when the server should exit.
            // It is safe to register for the same signal multiple times in a program,
            // provided all registration for the specified signal is made through Asio.
            m_signals.add(SIGINT);
            m_signals.add(SIGTERM);
#if defined(SIGQUIT)
            m_signals.add(SIGQUIT);
#endif // defined(SIGQUIT)

            m_signals.async_wait([this](boost::system::error_code /*ec*/, int signo)
                                 {
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
                const float maxIdleTime = CUserDefaults::instance().getOptions().m_server.m_idleTime;

                CMonitoringThread::instance().start(maxIdleTime,
                                                    []()
                                                    {
                                                        LOG(MiscCommon::info) << "Idle callback called.";
                                                    });
                m_acceptor.listen();

                createClientAndStartAccept();

                // Create a server info file
                createInfoFile();

                // a thread pool for the DDS transport engine
                // may return 0 when not able to detect
                unsigned int concurrentThreads = (0 == _nThreads) ? std::thread::hardware_concurrency() : _nThreads;
                // we need at least 2 threads
                if (concurrentThreads < 2)
                    concurrentThreads = 2;
                LOG(MiscCommon::info) << "Starting DDS transport engine using " << concurrentThreads
                                      << " concurrent threads.";
                for (int x = 0; x < concurrentThreads; ++x)
                {
                    m_workerThreads.create_thread([this]()
                                                  {
                                                      runService(10);
                                                  });
                }
                if (_join)
                    m_workerThreads.join_all();
            }
            catch (std::exception& e)
            {
                LOG(MiscCommon::fatal) << e.what();
            }
        }

        void runService(short _counter)
        {
            if (_counter <= 0)
            {
                LOG(MiscCommon::error) << "CConnectionManagerImpl: can't start another io_service.";
            }
            try
            {
                m_acceptor.get_io_service().run();
            }
            catch (std::exception& ex)
            {
                LOG(MiscCommon::error) << "CConnectionManagerImpl exception: " << ex.what();
                LOG(MiscCommon::info) << "CConnectionManagerImpl restarting io_service";
                runService(--_counter);
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

                auto condition = [](typename T::connectionPtr_t _v)
                {
                    return (_v->started());
                };

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

                m_acceptor.close();
                m_acceptor.get_io_service().stop();

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
        typename T::weakConnectionPtr_t getWeakPtr(T* _client)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            for (auto& v : m_channels)
            {
                if (v.get() == _client)
                    return v;
            }
            return typename T::weakConnectionPtr_t();
        }

        typename T::weakConnectionPtrVector_t getChannels(
            std::function<bool(typename T::connectionPtr_t)> _condition = nullptr)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            typename T::weakConnectionPtrVector_t result;
            result.reserve(m_channels.size());
            for (auto& v : m_channels)
            {
                if (_condition == nullptr || _condition(v))
                {
                    result.push_back(v);
                }
            }
            return result;
        }

        template <ECmdType _cmd, class AttachmentType>
        void broadcastMsg(const AttachmentType& _attachment,
                          std::function<bool(typename T::connectionPtr_t)> _condition = nullptr)
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
                                      std::function<bool(typename T::connectionPtr_t)> _condition = nullptr)
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
        void broadcastSimpleMsg(std::function<bool(typename T::connectionPtr_t)> _condition = nullptr)
        {
            SEmptyCmd cmd;
            broadcastMsg<_cmd>(cmd, _condition);
        }

        void broadcastBinaryAttachmentCmd(const MiscCommon::BYTEVector_t& _data,
                                          const std::string& _fileName,
                                          uint16_t _cmdSource,
                                          std::function<bool(typename T::connectionPtr_t)> _condition = nullptr)
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

        size_t countNofChannels(std::function<bool(typename T::connectionPtr_t)> _condition = nullptr)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (_condition == nullptr)
                return m_channels.size();
            size_t counter = 0;
            for (auto& v : m_channels)
            {
                if (_condition(v))
                {
                    counter++;
                }
            }
            return counter;
        }

      private:
        void acceptHandler(typename T::connectionPtr_t _client, const boost::system::error_code& _ec)
        {
            if (!_ec)
            {
                _client->start();
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_channels.push_back(_client);
                }
                createClientAndStartAccept();
            }
            else
            {
                LOG(MiscCommon::error) << "Can't accept new connection: " << _ec.message();
            }
        }

        void createClientAndStartAccept()
        {
            typename T::connectionPtr_t newClient = T::makeNew(m_acceptor.get_io_service());

            A* pThis = static_cast<A*>(this);
            pThis->newClientCreated(newClient);

            // Subscribe on dissconnect event
            newClient->registerDisconnectEventHandler([this](T* _channel) -> void
                                                      {
                                                          return this->removeClient(_channel);
                                                      });

            m_acceptor.async_accept(
                newClient->socket(),
                std::bind(&CConnectionManagerImpl::acceptHandler, this, newClient, std::placeholders::_1));
        }

        void createInfoFile()
        {
            // The child needs to have that method
            A* pThis = static_cast<A*>(this);
            pThis->_createInfoFile(m_acceptor.local_endpoint().port());
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
            LOG(MiscCommon::debug) << "Removing " /*<< _client->getTypeName()*/ << " client from the list of active";
            std::lock_guard<std::mutex> lock(m_mutex);
            m_channels.erase(remove_if(m_channels.begin(),
                                       m_channels.end(),
                                       [&](typename T::connectionPtr_t& i)
                                       {
                                           return (i.get() == _client);
                                       }),
                             m_channels.end());
        }

      private:
        boost::asio::ip::tcp::acceptor m_acceptor;
        boost::asio::ip::tcp::endpoint m_endpoint;
        /// The signal_set is used to register for process termination notifications.
        boost::asio::signal_set m_signals;
        std::mutex m_mutex;
        typename T::connectionPtrVector_t m_channels;
        boost::thread_group m_workerThreads;
    };
}
#endif /* defined(__DDS__ConnectionManagerImpl__) */
