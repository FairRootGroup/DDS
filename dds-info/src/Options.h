// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef _DDS_OPTIONS_H_
#define _DDS_OPTIONS_H_
//=============================================================================
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
// DDS
#include "ProtocolCommands.h"
#include "Version.h"
// STD
#include <string>
//=============================================================================
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    namespace info_cmd
    {
        /// \brief `dds-info` command line options
        typedef struct SOptions
        {
            bool m_bNeedCommanderPid{ false };
            bool m_bNeedStatus{ false };
            bool m_bNeedAgentList{ false };
            bool m_bNeedSlotList{ false };
            bool m_bNeedActiveTopology{ false };
            bool m_bNeedActiveCount{ false };
            bool m_bNeedIdleCount{ false };
            bool m_bNeedExecutingCount{ false };
            bool m_bHelp{ false };
            bool m_bVersion{ false };
            uint32_t m_nWaitCount{ 0 };
            boost::uuids::uuid m_sid{ boost::uuids::nil_uuid() };
        } SOptions_t;

        // Command line parser
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options)
        {
            if (nullptr == _options)
                throw std::runtime_error("Internal error: options' container is empty.");

            // Generic options
            bpo::options_description options("dds-info options");
            options.add_options()("help,h", bpo::bool_switch(&_options->m_bHelp), "Produce help message");
            options.add_options()("version,v", bpo::bool_switch(&_options->m_bVersion), "Version information");
            options.add_options()("session,s", bpo::value<std::string>(), "DDS Session ID");
            options.add_options()("commander-pid",
                                  bpo::bool_switch(&_options->m_bNeedCommanderPid),
                                  "Return the pid of the commander server");
            options.add_options()(
                "status", bpo::bool_switch(&_options->m_bNeedStatus), "Query current status of DDS commander server");
            options.add_options()("agent-list,l",
                                  bpo::bool_switch(&_options->m_bNeedAgentList),
                                  "Show detailed info about all online agents");
            options.add_options()(
                "slot-list", bpo::bool_switch(&_options->m_bNeedSlotList), "Show detailed info about all online slots");
            options.add_options()("active-count,n",
                                  bpo::bool_switch(&_options->m_bNeedActiveCount),
                                  "Returns a number of online agents.");
            options.add_options()(
                "idle-count", bpo::bool_switch(&_options->m_bNeedIdleCount), "Returns a number of idle agents.");
            options.add_options()("executing-count",
                                  bpo::bool_switch(&_options->m_bNeedExecutingCount),
                                  "Returns a number of executing agents.");
            options.add_options()("wait",
                                  bpo::value<uint32_t>(&_options->m_nWaitCount),
                                  "The command will block infinitely until a required number of agents are available. "
                                  "Must be used together with --active-count, --idle-count or --executing-count");
            options.add_options()("active-topology",
                                  bpo::bool_switch(&_options->m_bNeedActiveTopology),
                                  "Returns the name of the active topology");

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
            bpo::notify(vm);

            // check for non-defaulted arguments
            auto found{ find_if(vm.begin(),
                                vm.end(),
                                [](const bpo::variables_map::value_type& _v) { return (!_v.second.defaulted()); }) };

            if (vm.count("help") || vm.end() == found)
            {
                LOG(dds::misc::log_stdout) << options;
                return true;
            }
            if (vm.count("version"))
            {
                LOG(dds::misc::log_stdout) << dds::misc::DDSVersionInfoString();
                return true;
            }
            if (vm.count("session"))
                _options->m_sid = boost::uuids::string_generator()(vm["session"].as<std::string>());

            int numCounters{ _options->m_bNeedActiveCount + _options->m_bNeedIdleCount +
                             _options->m_bNeedExecutingCount };
            if (numCounters > 1)
            {
                LOG(dds::misc::log_stderr) << "--active-count, --idle-count, --executing-count can't be used together.";
                return false;
            }

            bool needCount{ _options->m_bNeedActiveCount || _options->m_bNeedIdleCount ||
                            _options->m_bNeedExecutingCount };
            if (vm.count("wait") && !needCount)
            {
                LOG(dds::misc::log_stderr)
                    << "Option --wait must be used together with --active-count, --idle-count or --executing-count.";
                return false;
            }

            if (vm.count("wait") && _options->m_nWaitCount <= 0)
            {
                LOG(dds::misc::log_stderr) << "A number of agents to wait must be higher than 0.";
                return false;
            }

            return true;
        }
    } // namespace info_cmd
} // namespace dds
#endif // _DDS_OPTIONS_H_
