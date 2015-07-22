// Copyright 2014 GSI, Inc. All rights reserved.
//
// Log engine core.
//
#ifndef LOGGER_H
#define LOGGER_H

// BOOST
#define BOOST_LOG_DYN_LINK
#include <boost/date_time/posix_time/posix_time_types.hpp>

// TODO: remove this warning suppression when BOOST 1.56 is released (when it is fixed there).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/current_process_id.hpp>
#include <boost/log/attributes/current_process_name.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#pragma clang diagnostic pop

// STD
#include <fstream>
#include <ostream>

// DDS
#include "def.h"
#include "UserDefaults.h"
#include "SysHelper.h"

// Main macro to be used for logging in DDS
// Example: LOG(info) << "My message";
#define LOG(severity) BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), severity)
// Convenience functions
#define P_H BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::proto_high)
#define P_M BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::proto_mid)
#define P_L BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::proto_low)
#define DBG BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::debug)
#define INF BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::info)
#define WRN BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::warning)
#define ERR BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::error)
#define FAT BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::fatal)
#define STDOUT BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::log_stdout)
#define STDOUT_CLEAN BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::log_stdout_clean)
#define STDERR BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::log_stderr)

namespace MiscCommon
{
    BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", ELogSeverityLevel)

    class Logger
    {
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

            std::string sLogFile = userDefaults.getLogFile();

            unsigned int rotationSize = userDefaults.getOptions().m_server.m_logRotationSize;
            unsigned int severityLevel = userDefaults.getOptions().m_server.m_logSeverityLevel;
            unsigned int hasConsoleOutput = userDefaults.getOptions().m_server.m_logHasConsoleOutput;

            // Default format for logger
            boost::log::formatter formatter =
                // TODO: std::setw doesn't work for the first collumn of the log (TimeStamp). Investigate!
                expressions::stream << std::left << expressions::format_date_time<boost::posix_time::ptime>(
                                                        "TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                                    << "   " << std::setw(7) << expressions::attr<ELogSeverityLevel>("Severity")
                                    << std::setw(20) << expressions::attr<std::string>("Process") << " <"
                                    << expressions::attr<attributes::current_process_id::value_type>("ProcessID") << ":"
                                    << expressions::attr<attributes::current_thread_id::value_type>("ThreadID")
                                    << ">    " << expressions::smessage;

            // Logging to file
            boost::shared_ptr<sinks::synchronous_sink<sinks::text_file_backend>> fileSink =
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

            // Logging to console
            boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend>> consoleSTDOUTSink =
                add_console_log(std::cout, boost::log::keywords::format = "%Process%: %Message%");
            boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend>> consoleSTDOUTCleanSink =
                add_console_log(std::cout, boost::log::keywords::format = "%Message%");
            boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend>> consoleSTDERRSink =
                add_console_log(std::cerr, boost::log::keywords::format = "%Process%: error: %Message%");

            consoleSTDOUTSink->set_filter(severity == log_stdout && hasConsoleOutput);
            consoleSTDOUTCleanSink->set_filter(severity == log_stdout_clean && hasConsoleOutput);
            consoleSTDERRSink->set_filter(severity == log_stderr && hasConsoleOutput);

            add_common_attributes();
            core::get()->add_global_attribute("Process", attributes::current_process_name());

            LOG(info) << "Log engine is initialized with severety \""
                      << userDefaults.getOptions().m_server.m_logSeverityLevel << "\"";
        }

      private:
        logger_t fLogger; ///> Main logger object
    };
};
#endif
