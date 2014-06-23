// Copyright 2014 GSI, Inc. All rights reserved.
//
// Log engine core.
//
#ifndef LOGGER_H
#define LOGGER_H

// BOOST
#define BOOST_LOG_DYN_LINK
#include <boost/date_time/posix_time/posix_time_types.hpp>
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

// STD
#include <fstream>

// DDS
#include "version.h"
#include "UserDefaults.h"
#include "SysHelper.h"

// Main macro to be used for logging in DDS
// Example: LOG(trace) << "My message";
#define LOG(severity) BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), severity)
// Convenience functions
#define TRACE BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::trace)
#define DEBUG BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::debug)
#define INFO BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::info)
#define WARNING BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::warning)
#define ERROR BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::error)
#define FATAL BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::fatal)
#define CONSOLE BOOST_LOG_SEV(MiscCommon::Logger::instance().logger(), MiscCommon::console)

namespace MiscCommon
{
    /// Severity levels
    enum ELogSeverityLevel
    {
        trace = 0,
        debug = 1,
        info = 2,
        warning = 3,
        error = 4,
        fatal = 5,
        console = 6
    };

    /// The operator puts a human-friendly representation of the severity level to the stream
    inline std::ostream& operator<<(std::ostream& strm, ELogSeverityLevel level)
    {
        static const char* strings[] = { "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL", "CONSOLE" };

        if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
            strm << strings[level];
        else
            strm << static_cast<int>(level);

        return strm;
    }

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
            using namespace boost::log;

            // Get the path to log file
            dds::CUserDefaults userDefaults;
            std::string sCfgFile(dds::CUserDefaults::currentUDFile());
            userDefaults.init(sCfgFile);
            std::string sLogDir(userDefaults.getOptions().m_general.m_logDir);
            smart_append<std::string>(&sLogDir, '/');
            std::string sLogFile(sLogDir);
            sLogFile += std::string(PROJECT_NAME) + ".log";
            smart_path<std::string>(&sLogFile);

            unsigned int rotationSize = userDefaults.getOptions().m_general.m_logRotationSize;
            unsigned int severityLevel = userDefaults.getOptions().m_general.m_logSeverityLevel;
            unsigned int hasConsoleOutput = userDefaults.getOptions().m_general.m_logHasConsoleOutput;

            // Default format for logger
            boost::log::formatter formatter = expressions::stream
                                              << expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f <")
                                              << expressions::attr<ELogSeverityLevel>("Severity") << "> [" << expressions::attr<std::string>("Process") << "] <"
                                              << expressions::attr<attributes::current_process_id::value_type>("ProcessID") << ":"
                                              << expressions::attr<attributes::current_thread_id::value_type>("ThreadID") << "> " << expressions::smessage;

            // Logging to file
            boost::shared_ptr<sinks::synchronous_sink<sinks::text_file_backend>> fileSink =
                add_file_log(keywords::file_name = sLogFile, keywords::open_mode = (std::ios::out | std::ios::app), keywords::rotation_size = rotationSize);
            fileSink->set_formatter(formatter);
            fileSink->locked_backend()->auto_flush(true);

            // Logging to console
            boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend>> consoleSink = add_console_log();
            consoleSink->set_filter((severity <= severityLevel && hasConsoleOutput) || severity == console);
            // consoleSink->set_formatter(formatter);

            add_common_attributes();
            core::get()->add_global_attribute("Process", attributes::current_process_name());
        }

      private:
        /// \brief Constructor
        //        Logger()
        //        {
        //            init();
        //        }

        logger_t fLogger; ///> Main logger object
    };
};
#endif
