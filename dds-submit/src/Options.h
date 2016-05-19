// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDSOPTIONS_H
#define DDSOPTIONS_H
//=============================================================================
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/property_tree/ptree.hpp>

// silence "Unused typedef" warning using clang 3.7+ and boost < 1.59
#if BOOST_VERSION < 105900
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#include <boost/property_tree/ini_parser.hpp>
#if BOOST_VERSION < 105900
#pragma clang diagnostic pop
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
// DDS
#include "BOOSTHelper.h"
#include "MiscUtils.h"
#include "ProtocolCommands.h"
#include "Res.h"
#include "SubmitCmd.h"
#include "SysHelper.h"
#include "version.h"

namespace bpo = boost::program_options;

namespace dds
{
    namespace submit_cmd
    {
        /// \brief dds-commander's container of options
        typedef struct SOptions
        {
            SOptions()
                : m_sRMS("localhost")
                , m_sCfgFile()
                , m_sPath()
                , m_number(0)
                , m_bListPlugins(false)
            {
            }

            std::string m_sRMS;
            std::string m_sCfgFile;
            std::string m_sPath;
            size_t m_number;
            bool m_bListPlugins;
        } SOptions_t;
        //=============================================================================
        inline std::ostream& operator<<(std::ostream& _stream, const SOptions& val)
        {
            return _stream << "\nRMS: " << val.m_sRMS << "\nPlug-in's configuration file: " << val.m_sCfgFile;
        }
        //=============================================================================
        inline void PrintVersion()
        {
            LOG(MiscCommon::log_stdout) << " v" << PROJECT_VERSION_STRING << "\n"
                                        << "DDS configuration"
                                        << " v" << USER_DEFAULTS_CFG_VERSION << "\n" << MiscCommon::g_cszReportBugsAddr;
        }
        //=============================================================================
        // Command line parser
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options) throw(std::exception)
        {
            if (nullptr == _options)
                throw std::runtime_error("Internal error: options' container is empty.");

            // Generic options
            bpo::options_description options("dds-submit options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()(
                "list,l", bpo::bool_switch(&_options->m_bListPlugins), "List all available RMS plug-ins");
            options.add_options()("rms,r",
                                  bpo::value<std::string>(&_options->m_sRMS),
                                  "Defines a destination resource "
                                  "management system plug-in. Use "
                                  "\"--list\" to find out names "
                                  "of available RMS plug-ins.");
            options.add_options()("config,c",
                                  bpo::value<std::string>(&_options->m_sCfgFile),
                                  "A plug-in's configuration file. It can be used to provide additional RMS options");
            options.add_options()("path",
                                  bpo::value<std::string>(&_options->m_sPath),
                                  "A plug-in's directory search path. It can be used for external RMS plug-ins.");
            options.add_options()("number,n",
                                  bpo::value<size_t>(&_options->m_number),
                                  "Defines a number of agents to spawn."
                                  "If 0 is provided as an argument, then a number of available logical cores will be "
                                  "used.\nThis option can not be mixed with \"--config\"");

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
            bpo::notify(vm);

            MiscCommon::BOOSTHelper::conflicting_options(vm, "list", "number");
            MiscCommon::BOOSTHelper::conflicting_options(vm, "list", "rms");
            MiscCommon::BOOSTHelper::conflicting_options(vm, "list", "config");
            MiscCommon::BOOSTHelper::conflicting_options(vm, "config", "number");

            // check for non-defaulted arguments
            bpo::variables_map::const_iterator found =
                find_if(vm.begin(), vm.end(), [](const bpo::variables_map::value_type& _v) {
                    return (!_v.second.defaulted());
                });

            if (vm.count("help") || vm.end() == found)
            {
                LOG(MiscCommon::log_stdout) << options;
                return false;
            }
            // "rms" requires either "config" or "number"
            if (vm.count("rms") && !vm.count("config") && !vm.count("number"))
            {
                LOG(MiscCommon::log_stdout) << options;
                return false;
            }
            if (vm.count("version"))
            {
                PrintVersion();
                return false;
            }

            // RMS plug-ins are always lower cased
            boost::to_lower(_options->m_sRMS);

            // make absolute path
            if (!_options->m_sCfgFile.empty())
            {
                boost::filesystem::path pathCfgFile(_options->m_sCfgFile);
                _options->m_sCfgFile = boost::filesystem::absolute(pathCfgFile).string();
            }
            return true;
        }
    }
}
#endif
