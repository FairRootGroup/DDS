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
        ACTIVATE = 0
    };

    // A custom streamer to help boost program options to convert string options to ETopologyCmdType
    inline std::istream& operator>>(std::istream& _in, ETopologyCmdType& _topologyCmd)
    {
        std::string token;
        _in >> token;
        if (token == "activate")
            _topologyCmd = ETopologyCmdType::ACTIVATE;
        else
            throw bpo::invalid_option_value(token);
        return _in;
    }

    inline std::ostream& operator<<(std::ostream& _out, ETopologyCmdType& _topologyCmd)
    {
        switch (_topologyCmd)
        {
            case ETopologyCmdType::ACTIVATE:
                _out << "activate";
                break;
            case ETopologyCmdType::UNKNOWN:
                break;
        }
        return _out;
    }

    /// \brief dds-agent-cmd's container of options
    typedef struct SOptions
    {
        SOptions()
            : m_topologyCmd(ETopologyCmdType::UNKNOWN)
        {
        }

        ETopologyCmdType m_topologyCmd;
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
        options.add_options()("cmd,c", bpo::value<ETopologyCmdType>(&_options->m_topologyCmd), "Command to topology.");

        bpo::positional_options_description positional;
        positional.add("cmd", -1);

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(_argc, _argv).options(options).positional(positional).run(), vm);
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
        if (!vm.count("cmd"))
        {
            LOG(MiscCommon::log_stderr) << "specify a topology command"
                                        << "\n\n" << options;
            return false;
        }

        return true;
    }
}
#endif
