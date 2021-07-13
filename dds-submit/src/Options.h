// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDSOPTIONS_H
#define DDSOPTIONS_H
//=============================================================================
// BOOST
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
// DDS
#include "BoostHelper.h"
#include "ProtocolCommands.h"
#include "SubmitCmd.h"
#include "SysHelper.h"
#include "Version.h"

namespace bpo = boost::program_options;

namespace dds
{
    namespace submit_cmd
    {
        /// \brief dds-commander's container of options
        typedef struct SOptions
        {
            std::string m_sRMS{ "localhost" };
            std::string m_sCfgFile;
            std::string m_sPath;
            size_t m_number{ 0 };
            size_t m_slots{ 0 };
            bool m_bListPlugins{ false };
            boost::uuids::uuid m_sid = boost::uuids::nil_uuid();
        } SOptions_t;
        //=============================================================================
        inline std::ostream& operator<<(std::ostream& _stream, const SOptions& val)
        {
            return _stream << "\nRMS: " << val.m_sRMS << "\nPlug-in's configuration file: " << val.m_sCfgFile;
        }

        // Command line parser
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options)
        {
            if (nullptr == _options)
                throw std::runtime_error("Internal error: options' container is empty.");

            // Generic options
            bpo::options_description options("dds-submit options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()("session,s", bpo::value<std::string>(), "DDS Session ID");
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
                                  bpo::value<size_t>(&_options->m_number)->default_value(1),
                                  "Defines a number of agents to spawn."
                                  "If 0 is provided as an argument, then a number of available logical cores will be "
                                  "used.\n");
            options.add_options()(
                "slots", bpo::value<size_t>(&_options->m_slots), "Defines a number of task slots per agent.");

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
            bpo::notify(vm);

            dds::misc::conflicting_options(vm, "list", "rms");
            dds::misc::conflicting_options(vm, "list", "config");
            dds::misc::conflicting_options(vm, "list", "slots");

            // check for non-defaulted arguments
            bpo::variables_map::const_iterator found =
                find_if(vm.begin(),
                        vm.end(),
                        [](const bpo::variables_map::value_type& _v) { return (!_v.second.defaulted()); });

            if (vm.count("help") || vm.end() == found)
            {
                LOG(dds::misc::log_stdout) << options;
                return false;
            }
            // "rms" requires either "config" or "number"
            if (vm.count("rms") && !vm.count("config") && !vm.count("number") && !vm.count("slots"))
            {
                LOG(dds::misc::log_stderr) << "--rms options requires either --config, --number, or --slots";
                LOG(dds::misc::log_stdout) << options;
                return false;
            }
            if (vm.count("version"))
            {
                LOG(dds::misc::log_stdout) << dds::misc::DDSVersionInfoString();
                return false;
            }

            if (vm.count("session"))
                _options->m_sid = boost::uuids::string_generator()(vm["session"].as<std::string>());

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
    } // namespace submit_cmd
} // namespace dds
#endif
