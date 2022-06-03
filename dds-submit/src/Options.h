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
namespace fs = boost::filesystem;

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
            size_t m_minInstances{ 0 };
            size_t m_slots{ 0 };
            bool m_bListPlugins{ false };
            boost::uuids::uuid m_sid = boost::uuids::nil_uuid();
            std::string m_groupName;
            std::string m_submissionTag;
            std::string m_envCfgFilePath;
            bool m_bEnableOverbooking{ false };
            std::vector<std::string> m_inlineConfig;
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
            options.add_options()("help,h", "Produce help message.\n");
            options.add_options()("version,v", "Version information.\n");
            options.add_options()("session,s", bpo::value<std::string>(), "DDS Session ID.\n");
            options.add_options()(
                "list,l", bpo::bool_switch(&_options->m_bListPlugins), "List all available RMS plug-ins.\n");
            options.add_options()("rms,r",
                                  bpo::value<std::string>(&_options->m_sRMS),
                                  "Defines a destination resource "
                                  "management system plug-in. Use "
                                  "\"--list\" to find out names "
                                  "of available RMS plug-ins.\n");
            options.add_options()(
                "config,c",
                bpo::value<std::string>(&_options->m_sCfgFile),
                "A plug-in's configuration file. It can be used to provide additional RMS options. It should contain "
                "only RMS options. To define custom environment per agent, use --env-config.\n");
            options.add_options()("env-config,e",
                                  bpo::value<std::string>(&_options->m_envCfgFilePath),
                                  "A path to a user enironment script. Will be execeuted once per agent (valid for all "
                                  "task slots of the agent).\n");
            options.add_options()(
                "inline-config",
                bpo::value<std::vector<std::string>>(&_options->m_inlineConfig)->multitoken(),
                "Content of this string will be added to the RMS job configuration file as is. It can be specified "
                "multiple times, example: dds-submit -r slurm -n 5 --slots=10 --inline-config=\"#SBATCH "
                "--cpus-per-task=124\" --inline-config=\"#SBATCH --test=45\"");
            options.add_options()("path",
                                  bpo::value<std::string>(&_options->m_sPath),
                                  "A plug-in's directory search path. It can be used for external RMS plug-ins.");
            options.add_options()("number,n",
                                  bpo::value<size_t>(&_options->m_number)->default_value(1),
                                  "Defines a number of DDS agents to spawn.\n");
            options.add_options()("min-instances",
                                  bpo::value<size_t>(&_options->m_minInstances)->default_value(0),
                                  "Request that a minimum of \"--min-instances\" and maximum of \"--number\" agents to "
                                  "spawn. This option is RMS plug-in depended. At the moment only the slurm plug-in "
                                  "supports it. If set to 0, the minimum is ignored.\n");
            options.add_options()(
                "slots", bpo::value<size_t>(&_options->m_slots), "Defines a number of task slots per agent.");
            options.add_options()("group-name,g",
                                  bpo::value<std::string>(&_options->m_groupName)->default_value("common"),
                                  "Defines a group name of agents of this submission.\n");
            options.add_options()(
                "submission-tag,t",
                bpo::value<std::string>(&_options->m_submissionTag)->default_value("dds_agent_job"),
                "It can be used to define a submission tag. DDS RMS plug-ins will use this tag to name DDS RMS jobs "
                "and directories they create on the worker nodes.");

            options.add_options()(
                "enable-overbooking",
                bpo::bool_switch(&_options->m_bEnableOverbooking)->default_value(false),
                "The flag instructs DDS RMS plug-in to not specify any CPU requirement for RMS jobs. For example, the "
                "SLURM plug-in will not add the \"#SBATCH --cpus-per-task\" option to the job script. Otherwise "
                "DDS will try to require as many CPU per agent as tasks slots.");

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
            bpo::notify(vm);

            dds::misc::conflicting_options(vm, "list", "rms");
            dds::misc::conflicting_options(vm, "list", "config");
            dds::misc::conflicting_options(vm, "list", "env-config");
            dds::misc::conflicting_options(vm, "list", "inline-config");
            dds::misc::conflicting_options(vm, "list", "slots");
            dds::misc::conflicting_options(vm, "list", "group-name");
            dds::misc::conflicting_options(vm, "list", "submission-tag");

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

            if (vm.count("group-name"))
            {
                const unsigned int groupNameLimit{ 256 };
                const std::string groupNameNotAllowedSymb{ " `\"@#%^&*()+=[]{};:\\|,.<>/$!?\t\r" };
                if (_options->m_groupName.find_first_of(groupNameNotAllowedSymb) != std::string::npos ||
                    _options->m_groupName.size() > groupNameLimit)
                {
                    LOG(dds::misc::log_stderr)
                        << "The group-name option can't be longer than " << groupNameLimit
                        << " symbols and should not contain whitespaces or any special character such as: "
                        << groupNameNotAllowedSymb;
                    return false;
                }
            }

            if (vm.count("submission-tag"))
            {
                const unsigned int submissionTagLimit{ 256 };
                const std::string submissionTagNotAllowedSymb{ " `\"@#%^&*()+=[]{};:\\|,.<>/$!?\t\r" };
                if (_options->m_submissionTag.empty() ||
                    _options->m_submissionTag.find_first_of(submissionTagNotAllowedSymb) != std::string::npos ||
                    _options->m_submissionTag.size() > submissionTagLimit)
                {
                    LOG(dds::misc::log_stderr)
                        << "The submission-tag option can't be empty or longer than " << submissionTagLimit
                        << " symbols and should not contain whitespaces or any special character such as: "
                        << submissionTagNotAllowedSymb;
                    return false;
                }
            }

            if (vm.count("env-config"))
            {
                fs::path envCfg{ _options->m_envCfgFilePath };
                if (!fs::exists(envCfg))
                {
                    LOG(dds::misc::log_stderr) << "Can't find environment configuration file: " << envCfg.native();
                    return false;
                }
                envCfg = fs::canonical(envCfg);
                _options->m_envCfgFilePath = envCfg.native();
            }

            if (vm.count("number") && vm.count("min-instances") && _options->m_minInstances > _options->m_number)
            {
                LOG(dds::misc::log_stderr) << "Value of min-instances is bigger than \"--number\"";
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
    } // namespace submit_cmd
} // namespace dds
#endif
