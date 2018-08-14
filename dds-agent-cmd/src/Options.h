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
#include "BOOSTHelper.h"
#include "ProtocolCommands.h"
#include "Res.h"
#include "version.h"
//=============================================================================
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    enum class EAgentCmdType
    {
        UNKNOWN = -1,
        GETLOG = 0,
    };
    typedef std::map<EAgentCmdType, std::string> mapAgentCmdTypeCodes_t;
    const mapAgentCmdTypeCodes_t AgentCmdTypeCodeToString = { { EAgentCmdType::GETLOG, "getlog" } };

    // A custom streamer to help boost program options to convert string options to EAgentCmdType
    inline std::istream& operator>>(std::istream& _in, EAgentCmdType& _agentCmd)
    {
        std::string token;
        _in >> token;
        if (token == "getlog")
            _agentCmd = EAgentCmdType::GETLOG;
        else
            throw bpo::invalid_option_value(token);
        return _in;
    }

    inline std::ostream& operator<<(std::ostream& _out, EAgentCmdType& _agentCmd)
    {
        auto found = AgentCmdTypeCodeToString.find(_agentCmd);
        if (found != AgentCmdTypeCodeToString.end())
            _out << found->second;

        return _out;
    }

    namespace agent_cmd_cmd
    {
        /// \brief dds-agent-cmd's container of options
        typedef struct SOptions
        {
            SOptions()
                : m_sendCommandToAllAgents(false)
                , m_agentCmd(EAgentCmdType::UNKNOWN)
                , m_verbose(false)
                , m_sid(boost::uuids::nil_uuid())
            {
            }

            bool m_sendCommandToAllAgents;
            EAgentCmdType m_agentCmd;
            bool m_verbose;
            boost::uuids::uuid m_sid;
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
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options)
        {
            if (nullptr == _options)
                throw std::runtime_error("Internal error: options' container is empty.");

            // Generic options
            bpo::options_description options("dds-agent-cmd options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()("session,s", bpo::value<std::string>(), "DDS Session ID");
            options.add_options()("verbose", "Verbose output");
            options.add_options()(
                "command",
                bpo::value<EAgentCmdType>(&_options->m_agentCmd),
                "The command is a name of a dds-agent-cmd command."
                " Can be one of the following: getlog.\n"
                "For user's convenience it is allowed to call dds-agent-cmd without \"--command\" option"
                " by just specifying the command name directly, like:\ndds-agent-cmd getlog\n\n"
                "Commands:\n"
                "   getlog: \tRetrieve log files from worker nodes. Files will be saved in ~/.DDS/log/agents\n");
            options.add_options()("all,a", "Send command to all active agents");

            bpo::positional_options_description positional;
            positional.add("command", -1);

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
            if (vm.count("verbose"))
            {
                _options->m_verbose = true;
            }
            if (!vm.count("command") || _options->m_agentCmd == EAgentCmdType::UNKNOWN)
            {
                LOG(MiscCommon::log_stderr) << "Nothing to do. Please, specify a command"
                                            << "\n\n"
                                            << options;
                return false;
            }
            if (vm.count("all"))
            {
                _options->m_sendCommandToAllAgents = true;
            }
            if (vm.count("session"))
            {
                _options->m_sid = boost::uuids::string_generator()(vm["session"].as<std::string>());
            }

            return true;
        }
    } // namespace agent_cmd_cmd
} // namespace dds
#endif
