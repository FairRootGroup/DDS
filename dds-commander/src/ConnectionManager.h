// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__ConnectionManager__
#define __DDS__ConnectionManager__
// BOOST
#include <boost/asio.hpp>
// DDS
#include "MonitoringThread.h"
#include "Options.h"

#include "ProtocolMessage.h"
#include "AgentChannel.h"

namespace dds
{
    template <class T>
    class CConnectionManager
    {
      public:
        CConnectionManager(const SOptions_t& _options,
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

            m_signals.async_wait([this](boost::system::error_code /*ec*/, int /*signo*/)
                                 {
                                     // The server is stopped by cancelling all outstanding asynchronous
                                     // operations. Once all operations have finished the io_service::run()
                                     // call will exit.
                                     stop();
                                 });
        }

        ~CConnectionManager()
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
                                                    { LOG(MiscCommon::info) << "Idle callback called"; });
                //

                m_acceptor.listen();
                typename T::connectionPtr_t client = T::makeNew(m_acceptor.get_io_service());
                m_acceptor.async_accept(
                    client->socket(),
                    std::bind(&CConnectionManager::acceptHandler, this, client, std::placeholders::_1));

                // Create a server info file
                createServerInfoFile();

                m_acceptor.get_io_service().run();
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

      private:
        void acceptHandler(typename T::connectionPtr_t _client, const boost::system::error_code& _ec)
        {
            if (!_ec) // FIXME: Add proper error processing
            {
                _client->start();
                m_channels.push_back(_client);

                typename T::connectionPtr_t newClient = T::makeNew(m_acceptor.get_io_service());
                newClient->registerMessageHandler(cmdGET_LOG,
                                                  [this](const CProtocolMessage& _msg)->bool
                                                  { return this->getLogHandler(_msg); });
                m_acceptor.async_accept(
                    newClient->socket(),
                    std::bind(&CConnectionManager::acceptHandler, this, newClient, std::placeholders::_1));
            }
            else
            {
            }
        }

        void createServerInfoFile() const
        {
            const std::string sSrvCfg(CUserDefaults::instance().getServerInfoFile());
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
            const std::string sSrvCfg(CUserDefaults::instance().getServerInfoFile());
            if (sSrvCfg.empty())
                return;

            // TODO: check error code
            unlink(sSrvCfg.c_str());
        }

        bool getLogHandler(const CProtocolMessage& _msg)
        {
            // FIXME : temporary work around to get the working version.
            LOG(MiscCommon::debug) << "Call getLogHandler callback";
            for (const auto& v : m_channels)
            {

                CAgentChannel* channel = reinterpret_cast<CAgentChannel*>(v.get());
                if (channel->getType() == EAgentChannelType::AGENT)
                {
                    CProtocolMessage msg;
                    msg.encode<cmdGET_LOG>();
                    v->pushMsg(msg);
                }
                // v->stop(); // pushMsg<cmdGET_LOG>();
            }
            return true;
        }

      private:
        boost::asio::ip::tcp::acceptor m_acceptor;
        boost::asio::ip::tcp::endpoint m_endpoint;
        /// The signal_set is used to register for process termination notifications.
        boost::asio::signal_set m_signals;

        typename T::connectionPtrVector_t m_channels;
        dds::SOptions_t m_options;
    };
}
#endif /* defined(__DDS__ConnectionManager__) */
