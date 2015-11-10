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

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
// DDS
#include "version.h"
#include "Res.h"
#include "ProtocolCommands.h"
#include "SysHelper.h"
#include "MiscUtils.h"

namespace bpo = boost::program_options;

namespace dds
{
    namespace submit_cmd
    {
        /// \brief dds-commander's container of options
        typedef struct SOptions
        {
            SOptions()
                : m_RMS(protocol_api::SSubmitCmd::UNKNOWN)
                , m_number(0)
            {
            }

            std::string defaultConfigFile() const
            {
                std::string sWorkDir(dds::user_defaults_api::CUserDefaults::instance().getOptions().m_server.m_workDir);
                MiscCommon::smart_path(&sWorkDir);
                // We need to be sure that there is "/" always at the end of the path
                MiscCommon::smart_append<std::string>(&sWorkDir, '/');
                std::string sCfgFile(sWorkDir);
                sCfgFile += "dds-submit.cfg";
                return sCfgFile;
            }

            void load(std::string* _filename = nullptr)
            {
                std::string sCfgFile((_filename == nullptr) ? defaultConfigFile() : *_filename);

                // Create an empty property tree object
                using boost::property_tree::ptree;
                ptree pt;

                try
                {
                    LOG(MiscCommon::info) << "Loading options from: " << sCfgFile;
                    read_ini(sCfgFile, pt);

                    std::stringstream ssRMS;
                    ssRMS << pt.get<std::string>("dds-submit.RMS");
                    ssRMS >> m_RMS;

                    m_sSSHCfgFile = pt.get<std::string>("dds-submit.SSHPlugIn-ConfigFile");
                    LOG(MiscCommon::info) << *this;
                }
                catch (...)
                {
                    // ignore missing nodes
                }
            }
            void save(std::string* _filename = nullptr)
            {
                std::string sCfgFile((_filename == nullptr) ? defaultConfigFile() : *_filename);

                // Create an empty property tree object
                using boost::property_tree::ptree;
                ptree pt;

                std::stringstream ssRMS;
                ssRMS << m_RMS;
                pt.put("dds-submit.RMS", ssRMS.str());

                pt.put("dds-submit.SSHPlugIn-ConfigFile", m_sSSHCfgFile);

                // Write the property tree to the XML file.
                write_ini(sCfgFile, pt);
            }

            protocol_api::SSubmitCmd::ERmsType m_RMS;
            std::string m_sSSHCfgFile;
            size_t m_number;
        } SOptions_t;
        //=============================================================================
        inline std::ostream& operator<<(std::ostream& _stream, const SOptions& val)
        {
            return _stream << "\nRMS: " << val.m_RMS << "\nSSHPlugIn-ConfigFile" << val.m_sSSHCfgFile;
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

            // Init options from the default config file
            _options->load();

            // Generic options
            bpo::options_description options("dds-submit options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()("config,c", bpo::value<std::string>(), "A dds-submit configuration file.");
            options.add_options()("rms,r",
                                  bpo::value<dds::protocol_api::SSubmitCmd::ERmsType>(&_options->m_RMS),
                                  "Defines a destination resource management system. (default: ssh)\n"
                                  "Supported RMS:\n"
                                  "   - ssh\n"
                                  "   - localhost");
            options.add_options()("ssh-rms-cfg",
                                  bpo::value<std::string>(&_options->m_sSSHCfgFile),
                                  "A DDS's ssh plug-in configuration file. The option can only be used "
                                  "with the submit command when \'ssh\' is used as RMS");
            options.add_options()("number,n",
                                  bpo::value<size_t>(&_options->m_number),
                                  "Defines a number of agents to spawn. It can be used only when \'localhost\' is used "
                                  "as RMS. A number of available logical cores will be used if the parameter is "
                                  "omitted.");

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
            bpo::notify(vm);

            if (vm.count("config"))
            {
                // Init options from the given config file
                std::string sCfg(vm["config"].as<std::string>());
                MiscCommon::smart_path(&sCfg);
                if (!boost::filesystem::exists(sCfg))
                {
                    LOG(MiscCommon::log_stderr) << "Config file is missing: " << sCfg;
                    return false;
                }
                _options->load(&sCfg);
            }

            if (vm.count("help") || (_options->m_RMS == protocol_api::SSubmitCmd::UNKNOWN))
            {
                LOG(MiscCommon::log_stdout) << options;
                return false;
            }
            if (vm.count("version"))
            {
                PrintVersion();
                return false;
            }

            if (vm.count("rms") && (_options->m_RMS == protocol_api::SSubmitCmd::SSH && !vm.count("ssh-rms-cfg")))
            {
                LOG(MiscCommon::log_stderr) << "The SSH plug-in requires a rms configuration file. Please us "
                                               "--ssh-rms-cfg to specify a desired configuration file."
                                            << "\n\n" << options;
                return false;
            }

            if (vm.count("number") && (_options->m_RMS != protocol_api::SSubmitCmd::LOCALHOST))
            {
                LOG(MiscCommon::log_stderr) << "The \'number\' argument can only be used with \'localhost\' RMS"
                                            << "\n\n" << options;
                return false;
            }

            // make absolute path
            boost::filesystem::path pathSSHCfgFile(_options->m_sSSHCfgFile);
            _options->m_sSSHCfgFile = boost::filesystem::absolute(pathSSHCfgFile).string();

            // Save options to the config file
            _options->save();

            return true;
        }
    }
}
#endif
