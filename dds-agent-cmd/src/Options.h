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
#include "BOOSTHelper.h"
//=============================================================================
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    enum class EAgentCmdType
    {
        UNKNOWN = -1,
        GETLOG = 0,
        UPDATE_KEY
    };
    typedef std::map<EAgentCmdType, std::string> mapAgentCmdTypeCodes_t;
    const mapAgentCmdTypeCodes_t AgentCmdTypeCodeToString = { { EAgentCmdType::GETLOG, "getlog" },
                                                              { EAgentCmdType::UPDATE_KEY, "update-key" } };

    // A custom streamer to help boost program options to convert string options to EAgentCmdType
    inline std::istream& operator>>(std::istream& _in, EAgentCmdType& _agentCmd)
    {
        std::string token;
        _in >> token;
        if (token == "getlog")
            _agentCmd = EAgentCmdType::GETLOG;
        else if (token == "update-key")
            _agentCmd = EAgentCmdType::UPDATE_KEY;
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
            {
            }

            bool m_sendCommandToAllAgents;
            EAgentCmdType m_agentCmd;
            std::string m_sUpdKey_key;
            std::string m_sUpdKey_value;
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
            options.add_options()("verbose", "Verbose output");
            options.add_options()(
                "command",
                bpo::value<EAgentCmdType>(&_options->m_agentCmd),
                "The command is a name of a dds-agent-cmd command."
                " Can be one of the following: getlog, and update-key.\n"
                "For user's convenience it is allowed to call dds-agent-cmd without \"--command\" option"
                " by just specifying the command name directly, like:\ndds-agent-cmd getlog\n\n"
                "Commands:\n"
                "   getlog: \tRetrieve log files from worker nodes. Files will be saved in ~/.DDS/log/agents\n"
                "   update-key: \tIt forces an update of a given task's property in the topology. Name of the property "
                "and "
                "its value should be provided additionally (see --key and --value) \n");
            options.add_options()("all,a", "Send command to all active agents");
            options.add_options()(
                "key", bpo::value<std::string>(&_options->m_sUpdKey_key), "Specefies the key to update");
            options.add_options()("value",
                                  bpo::value<std::string>(&_options->m_sUpdKey_value),
                                  "Specefies the new value ofthe given key");

            bpo::positional_options_description positional;
            positional.add("command", -1);

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).positional(positional).run(), vm);
            bpo::notify(vm);

            MiscCommon::BOOSTHelper::option_dependency(vm, "key", "value");

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
                                            << "\n\n" << options;
                return false;
            }
            if (EAgentCmdType::UPDATE_KEY == _options->m_agentCmd && !vm.count("key"))
            {
                LOG(MiscCommon::log_stderr) << "Please, specify the key to update"
                                            << "\n\n" << options;
                return false;
            }
            if (vm.count("all"))
            {
                _options->m_sendCommandToAllAgents = true;
            }

            return true;
        }
    }
}
#endif
