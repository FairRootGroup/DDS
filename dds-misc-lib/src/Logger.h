// Copyright 2014 GSI, Inc. All rights reserved.
//
// Log engine core.
//
#ifndef _DDS_LOGGER_H_
#define _DDS_LOGGER_H_

// BOOST
#ifndef BOOST_LOG_DYN_LINK
#define BOOST_LOG_DYN_LINK
#endif
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/attributes/current_process_id.hpp>
#include <boost/log/attributes/current_process_name.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
// STD
#include <fstream>
#include <ostream>

// DDS
#include "SysHelper.h"
#include "UserDefaults.h"
#include "def.h"

#define LOG(severity) BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), severity)

// Convenience functions
#define P_H BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), dds::misc::proto_high)
#define P_M BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), dds::misc::proto_mid)
#define P_L BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), dds::misc::proto_low)
#define DBG BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), dds::misc::debug)
#define INF BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), dds::misc::info)
#define WRN BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), dds::misc::warning)
#define ERR BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), dds::misc::error)
#define FAT BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), dds::misc::fatal)
#define STDOUT BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), dds::misc::log_stdout)
#define STDOUT_CLEAN BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), dds::misc::log_stdout_clean)
#define STDERR BOOST_LOG_SEV(dds::misc::Logger::instance().logger(), dds::misc::log_stderr)

namespace dds::misc
{
    BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", ELogSeverityLevel)

    class Logger
    {
      private:
        typedef boost::shared_ptr<boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend>> fileSink_t;

      public:
        typedef boost::log::sources::severity_logger_mt<ELogSeverityLevel> logger_t;

        /// \brief Return singleton instance
        static Logger& instance()
        {
            static Logger instance;
            return instance;
        }

        logger_t& logger()
        {
            return fLogger;
        }

        /// \brief Initialization of log. Has to be called in main.
        void init()
        {
            static bool bStarted = false;
            if (bStarted)
                return;

            bStarted = true;

            using namespace boost::log;

            const dds::user_defaults_api::CUserDefaults& userDefaults =
                dds::user_defaults_api::CUserDefaults::instance();

            unsigned int hasConsoleOutput = userDefaults.getOptions().m_server.m_logHasConsoleOutput;

            m_fileSink = createFileSink();

            // Logging to console
            typedef boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend>> ostreamSink_t;
            ostreamSink_t stdoutSink = add_console_log(std::cout, keywords::format = "%Process%: %Message%");
            ostreamSink_t stdoutCleanSink = add_console_log(std::cout, keywords::format = "%Message%");
            ostreamSink_t stderrSink = add_console_log(std::cerr, keywords::format = "%Process%: error: %Message%");

            stdoutSink->set_filter(severity == log_stdout && hasConsoleOutput);
            stdoutCleanSink->set_filter(severity == log_stdout_clean && hasConsoleOutput);
            stderrSink->set_filter(severity == log_stderr && hasConsoleOutput);

            add_common_attributes();
            core::get()->add_global_attribute("Process", attributes::current_process_name());

            LOG(info) << "Log engine is initialized with severety \""
                      << userDefaults.getOptions().m_server.m_logSeverityLevel << "\"";
            LOG(info) << getpid() << " (process ID) : " << getpgrp() << " (process group ID) : " << getppid()
                      << "(parent process ID)";
        }

        void reinit()
        {
            boost::log::core::get()->remove_sink(m_fileSink);
            m_fileSink.reset();
            m_fileSink = createFileSink();
        }

      private:
        fileSink_t createFileSink() const
        {
            using namespace boost::log;

            const dds::user_defaults_api::CUserDefaults& userDefaults =
                dds::user_defaults_api::CUserDefaults::instance();
            unsigned int severityLevel = userDefaults.getOptions().m_server.m_logSeverityLevel;
            unsigned int rotationSize = userDefaults.getOptions().m_server.m_logRotationSize;
            std::string sLogFile = userDefaults.getLogFile();

            // Default format for logger
            formatter formatter =
                // TODO: std::setw doesn't work for the first collumn of the log (TimeStamp). Investigate!
                expressions::stream << std::left
                                    << expressions::format_date_time<boost::posix_time::ptime>("TimeStamp",
                                                                                               "%Y-%m-%d %H:%M:%S.%f")
                                    << "   " << std::setw(7) << expressions::attr<ELogSeverityLevel>("Severity")
                                    << std::setw(20) << expressions::attr<std::string>("Process") << " <"
                                    << expressions::attr<attributes::current_process_id::value_type>("ProcessID") << ":"
                                    << expressions::attr<attributes::current_thread_id::value_type>("ThreadID")
                                    << ">    " << expressions::smessage;

            fileSink_t fileSink =
                add_file_log(keywords::file_name = sLogFile,
                             keywords::open_mode = (std::ios::out | std::ios::app),
                             keywords::rotation_size = rotationSize * 1024 * 1024,
                             // rotate at midnight every day
                             keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
                             // log collector,
                             // -- maximum total size of the stored log files is 1GB.
                             // -- minimum free space on the drive is 2GB
                             keywords::max_size = 1000 * 1024 * 1024,
                             keywords::min_free_space = 2000 * 1024 * 1024,
                             keywords::auto_flush = true);

            fileSink->set_formatter(formatter);
            fileSink->set_filter(severity >= severityLevel);

            return fileSink;
        }

      private:
        logger_t fLogger; ///> Main logger object

        fileSink_t m_fileSink; ///> File sink
    };
};     // namespace dds::misc
#endif //_DDS_LOGGER_H_
