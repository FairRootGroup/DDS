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
        CConnectionManagerImpl(const SOptions_t& _options,
                               boost::asio::io_service& _io_service,
                               boost::asio::ip::tcp::endpoint& _endpoint)
            : m_acceptor(_io_service, _endpoint)
            , m_signals(_io_service)
            , m_options(_options)
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
                                     LOG(MiscCommon::info) << "Sopping DDS commander server";

                                     stop();
                                 });
        }

        ~CConnectionManagerImpl()
        {
            // Delete server info file
            deleteServerInfoFile();
        }

        void start()
        {
            try
            {
                // Start monitoring thread
                const float maxIdleTime = CUserDefaults::instance().getOptions().m_server.m_idleTime;

                CMonitoringThread::instance().start(maxIdleTime,
                                                    []()
                                                    {
                    LOG(MiscCommon::info) << "Idle callback called";
                });
                m_acceptor.listen();

                createClientAndStartAccept();

                // Create a server info file
                createServerInfoFile();

                // a thread pool for the DDS transport engine
                // may return 0 when not able to detect
                unsigned int concurrentThreads = std::thread::hardware_concurrency();
                // we need at least 4 threads
                if (concurrentThreads < 4)
                    concurrentThreads = 4;
                boost::thread_group worker_threads;
                LOG(MiscCommon::info) << "Starting DDS transport engine using " << concurrentThreads
                                      << " concurrent threads.";
                for (int x = 0; x < concurrentThreads; ++x)
                {
                    worker_threads.create_thread(
                        boost::bind(&boost::asio::io_service::run, &(m_acceptor.get_io_service())));
                }
                worker_threads.join_all();
            }
            catch (std::exception& e)
            {
                LOG(MiscCommon::fatal) << e.what();
            }
        }

        void stop()
        {
            try
            {
                // TODO: Use mutex
                // Send shutdown signal to all client connections.
                for (const auto& v : m_channels)
                {
                    CProtocolMessage::protocolMessagePtr_t msg = std::make_shared<CProtocolMessage>();
                    msg->encode<cmdSHUTDOWN>();
                    v->syncPushMsg(msg);
                }

                m_acceptor.close();
                m_acceptor.get_io_service().stop();
                for (const auto& v : m_channels)
                {
                    v->stop();
                }
                m_channels.clear();
            }
            catch (std::exception& e)
            {
                LOG(MiscCommon::fatal) << e.what();
            }
        }

      protected:
        typename T::weakConnectionPtr_t useRawPtr(T* _client) const
        {
            // TODO: Use mutex
            for (auto& v : m_channels)
            {
                if (v.get() == _client)
                    return v;
            }
            return typename T::weakConnectionPtr_t();
        }

        typename T::weakConnectionPtrVector_t getChannels(
            std::function<bool(typename T::connectionPtr_t)> _condition = nullptr) const
        {
            // TODO: Use mutex
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

        void broadcastMsg(CProtocolMessage::protocolMessagePtr_t _msg,
                          std::function<bool(typename T::connectionPtr_t)> _condition = nullptr)
        {
            for (auto& v : m_channels)
            {
                if (_condition == nullptr || _condition(v))
                {
                    v->pushMsg(_msg);
                }
            }
        }

        template <ECmdType _cmd>
        void broadcastMsg(std::function<bool(typename T::connectionPtr_t)> _condition = nullptr)
        {
            CProtocolMessage::protocolMessagePtr_t msg = std::make_shared<CProtocolMessage>();
            msg->encode<_cmd>();
            broadcastMsg(msg, _condition);
        }

        size_t countNofChannels(std::function<bool(typename T::connectionPtr_t)> _condition = nullptr)
        {
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
            if (!_ec) // FIXME: Add proper error processing
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
            }
        }

        void createClientAndStartAccept()
        {
            typename T::connectionPtr_t newClient = T::makeNew(m_acceptor.get_io_service());

            A* pThis = static_cast<A*>(this);
            pThis->newClientCreated(newClient);

            // Subscribe on dissconnect event
            newClient->registerDissconnectEventHandler([this](T* _channel) -> void
                                                       {
                return this->removeClient(_channel);
            });

            m_acceptor.async_accept(
                newClient->socket(),
                std::bind(&CConnectionManagerImpl::acceptHandler, this, newClient, std::placeholders::_1));
        }

        void createServerInfoFile() const
        {
            const std::string sSrvCfg(CUserDefaults::instance().getServerInfoFileLocationSrv());
            LOG(MiscCommon::info) << "Creating a server info file: " << sSrvCfg;
            std::ofstream f(sSrvCfg.c_str());
            if (!f.is_open() || !f.good())
            {
                std::string msg("Could not open a server info configuration file: ");
                msg += sSrvCfg;
                throw std::runtime_error(msg);
            }

            std::string srvHost;
            MiscCommon::get_hostname(&srvHost);
            std::string srvUser;
            MiscCommon::get_cuser_name(&srvUser);

            f << "[server]\n"
              << "host=" << srvHost << "\n"
              << "user=" << srvUser << "\n"
              << "port=" << m_acceptor.local_endpoint().port() << "\n" << std::endl;
        }

        void deleteServerInfoFile() const
        {
            const std::string sSrvCfg(CUserDefaults::instance().getServerInfoFileLocationSrv());
            if (sSrvCfg.empty())
                return;

            // TODO: check error code
            unlink(sSrvCfg.c_str());
        }

        void removeClient(T* _client)
        {
            LOG(MiscCommon::debug) << "Removing agent from the list of active";
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

      protected:
        dds::SOptions_t m_options;
    };
}
#endif /* defined(__DDS__ConnectionManagerImpl__) */
