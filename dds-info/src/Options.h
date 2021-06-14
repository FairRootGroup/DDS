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
        /// \brief dds-commander's container of options
        typedef struct SOptions
        {
            SOptions()
                : m_bNeedCommanderPid(false)
                , m_bNeedDDSStatus(false)
                , m_bNeedAgentsList(false)
                , m_bNeedPropList(false)
                , m_bNeedPropValues(false)
                , m_bNeedActiveTopology(false)
                , m_propertyName()
                , m_sid(boost::uuids::nil_uuid())
                , m_bNeedActiveCount(false)
                , m_bNeedIdleCount(false)
                , m_bNeedExecutingCount(false)
                , m_nWaitCount(0)
            {
            }

            bool m_bNeedCommanderPid;
            bool m_bNeedDDSStatus;
            bool m_bNeedAgentsList;
            bool m_bNeedPropList;
            bool m_bNeedPropValues;
            bool m_bNeedActiveTopology;
            std::string m_propertyName;
            boost::uuids::uuid m_sid;
            bool m_bNeedActiveCount;
            bool m_bNeedIdleCount;
            bool m_bNeedExecutingCount;
            uint32_t m_nWaitCount;
        } SOptions_t;

        // Command line parser
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options)
        {
            if (nullptr == _options)
                throw std::runtime_error("Internal error: options' container is empty.");

            // Generic options
            bpo::options_description options("dds-info options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()("session,s", bpo::value<std::string>(), "DDS Session ID");
            options.add_options()("commander-pid",
                                  bpo::bool_switch(&_options->m_bNeedCommanderPid),
                                  "Return the pid of the commander server");
            options.add_options()("status",
                                  bpo::bool_switch(&_options->m_bNeedDDSStatus),
                                  "Query current status of DDS commander server");
            options.add_options()("agents-list,l",
                                  bpo::bool_switch(&_options->m_bNeedAgentsList),
                                  "Show detailed info about all online agents");
            options.add_options()(
                "prop-list", bpo::bool_switch(&_options->m_bNeedPropList), "Returns a property list from all agents.");
            options.add_options()("prop-values",
                                  bpo::bool_switch(&_options->m_bNeedPropValues),
                                  "Returns a key-value pairs from all agents.");
            options.add_options()("prop-name",
                                  bpo::value<std::string>(&_options->m_propertyName),
                                  "Specify property names that have to be returned.");
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
            bpo::variables_map::const_iterator found =
                find_if(vm.begin(),
                        vm.end(),
                        [](const bpo::variables_map::value_type& _v) { return (!_v.second.defaulted()); });

            if (vm.count("help") || vm.end() == found)
            {
                LOG(MiscCommon::log_stdout) << options;
                return false;
            }
            if (vm.count("version"))
            {
                LOG(MiscCommon::log_stdout) << MiscCommon::DDSVersionInfoString();
                return false;
            }
            if (vm.count("prop-name") && !_options->m_bNeedPropValues)
            {
                LOG(MiscCommon::log_stdout) << "Option prop-id has to be used together with prop-values.";
                return false;
            }

            if (vm.count("session"))
                _options->m_sid = boost::uuids::string_generator()(vm["session"].as<std::string>());

            size_t numCounters =
                _options->m_bNeedActiveCount + _options->m_bNeedIdleCount + _options->m_bNeedExecutingCount;
            if (numCounters > 1)
            {
                LOG(MiscCommon::log_stderr)
                    << "--active-count, --idle-count, --executing-count can't be used together.";
                return false;
            }

            bool needCount =
                _options->m_bNeedActiveCount || _options->m_bNeedIdleCount || _options->m_bNeedExecutingCount;
            if (vm.count("wait") && !needCount)
            {
                LOG(MiscCommon::log_stderr)
                    << "Option --wait must be used together with --active-count, --idle-count or --executing-count.";
                return false;
            }

            if (vm.count("wait") && _options->m_nWaitCount <= 0)
            {
                LOG(MiscCommon::log_stderr) << "A number of agents to wait must be higher than 0.";
                return false;
            }

            return true;
        }
    } // namespace info_cmd
} // namespace dds
#endif
