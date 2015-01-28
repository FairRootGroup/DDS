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
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
// DDS
#include "version.h"
#include "Res.h"
#include "ProtocolCommands.h"
//=============================================================================
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    enum class ETopologyCmdType
    {
        UNKNOWN = -1,
        ACTIVATE = 0,
        VALIDATE
    };

    /// \brief dds-agent-cmd's container of options
    typedef struct SOptions
    {
        SOptions()
            : m_topologyCmd(ETopologyCmdType::UNKNOWN)
            , m_sTopoFile()
            , m_verbose(false)
        {
        }

        ETopologyCmdType m_topologyCmd;
        std::string m_sTopoFile;
        bool m_verbose;
    } SOptions_t;
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
        bpo::options_description options("dds-agent-cmd options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("version,v", "Version information");
        options.add_options()("activate", "Request to activate agents, i.e. distribute and start user tasks.");
        options.add_options()(
            "validate", bpo::value<std::string>(&_options->m_sTopoFile), "Validate topology file against XSD schema.");
        options.add_options()("verbose", "Verbose output");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
        bpo::notify(vm);

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
        if (vm.count("validate") && !_options->m_sTopoFile.empty())
        {
            _options->m_topologyCmd = ETopologyCmdType::VALIDATE;
            // make absolute path
            boost::filesystem::path pathTopoFile(_options->m_sTopoFile);
            _options->m_sTopoFile = boost::filesystem::absolute(pathTopoFile).string();
            return true;
        }
        if (vm.count("activate"))
        {
            _options->m_topologyCmd = ETopologyCmdType::ACTIVATE;
            return true;
        }

        return true;
    }
}
#endif
