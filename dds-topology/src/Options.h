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
// DDS
#include "BOOSTHelper.h"
#include "ProtocolCommands.h"
#include "Res.h"
#include "version.h"
//=============================================================================
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    namespace topology_cmd
    {
        enum class ETopologyCmdType
        {
            UNKNOWN = -1,
            UPDATE = 0,
            ACTIVATE,
            STOP,
            VALIDATE
        };

        /// \brief dds-agent-cmd's container of options
        typedef struct SOptions
        {
            SOptions()
                : m_topologyCmd(ETopologyCmdType::UNKNOWN)
                , m_verbose(false)
                , m_bDisiableValidation(false)
            {
            }

            ETopologyCmdType m_topologyCmd;
            std::string m_sTopoFile;
            bool m_verbose;
            bool m_bDisiableValidation;
        } SOptions_t;
        //=============================================================================
        inline void PrintVersion()
        {
            LOG(MiscCommon::log_stdout) << " v" << PROJECT_VERSION_STRING << "\n"
                                        << "DDS configuration"
                                        << " v" << USER_DEFAULTS_CFG_VERSION << "\n"
                                        << MiscCommon::g_cszReportBugsAddr;
        }
        //=============================================================================
        // Command line parser
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options) throw(std::exception)
        {
            if (nullptr == _options)
                throw std::runtime_error("Internal error: options' container is empty.");

            // Generic options
            bpo::options_description options("dds-agent-cmd options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()("update,u",
                                  bpo::value<std::string>(&_options->m_sTopoFile),
                                  "Define a topology to update currently active topology.");
            options.add_options()("disable-validation",
                                  bpo::bool_switch(&_options->m_bDisiableValidation),
                                  "Disiable topology valiadation.");
            options.add_options()("activate",
                                  bpo::value<std::string>(&_options->m_sTopoFile),
                                  "Request to activate agents, i.e. distribute and start user tasks.");
            options.add_options()("stop", "Request to stop execution of user tasks.");
            options.add_options()("validate",
                                  bpo::value<std::string>(&_options->m_sTopoFile),
                                  "Validate topology file against XSD schema.");
            options.add_options()("verbose,V", "Verbose output");

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
            bpo::notify(vm);

            MiscCommon::BOOSTHelper::conflicting_options(vm, "activate", "stop");
            MiscCommon::BOOSTHelper::conflicting_options(vm, "update", "stop");
            MiscCommon::BOOSTHelper::conflicting_options(vm, "update", "activate");

            if (vm.count("help") || vm.empty())
            {
                LOG(MiscCommon::log_stdout) << options;
                return false;
            }
            if (vm.count("version"))
            {
                PrintVersion();
                return false;
            }
            if (vm.count("verbose"))
            {
                _options->m_verbose = true;
            }
            if (vm.count("disable-validation") && _options->m_bDisiableValidation)
            {
                if (!vm.count("activate") && !vm.count("update"))
                {
                    throw std::runtime_error("--disable-validation must be used together with --activate or --update");
                }
            }
            if (vm.count("validate") && !_options->m_sTopoFile.empty())
            {
                _options->m_topologyCmd = ETopologyCmdType::VALIDATE;
                // make absolute path
                boost::filesystem::path pathTopoFile(_options->m_sTopoFile);
                _options->m_sTopoFile = boost::filesystem::absolute(pathTopoFile).string();
                return true;
            }
            if (vm.count("update") || vm.count("activate"))
            {
                // check, that topo file exists
                if (!boost::filesystem::exists(_options->m_sTopoFile))
                {
                    std::string sMsg("Can't find the topo file: ");
                    sMsg += _options->m_sTopoFile;
                    throw std::runtime_error(sMsg);
                }

                if (vm.count("update"))
                {
                    _options->m_topologyCmd = ETopologyCmdType::UPDATE;
                }
                else if (vm.count("activate"))
                {
                    _options->m_topologyCmd = ETopologyCmdType::ACTIVATE;
                }
                // make absolute path
                boost::filesystem::path pathTopoFile(_options->m_sTopoFile);
                _options->m_sTopoFile = boost::filesystem::absolute(pathTopoFile).string();
                return true;
            }
            else if (vm.count("stop"))
            {
                _options->m_topologyCmd = ETopologyCmdType::STOP;
                _options->m_sTopoFile = "";
                return true;
            }
            else
            {
                LOG(MiscCommon::log_stdout) << options;
                return false;
            }

            return true;
        }
    }
}
#endif
