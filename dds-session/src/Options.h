// Copyright 2018 GSI, Inc. All rights reserved.
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
#include "BoostHelper.h"
#include "SysHelper.h"
#include "Version.h"

namespace bpo = boost::program_options;

namespace dds
{
    namespace session_cmd
    {
        struct SSessionsSorting
        {
            enum ETypes
            {
                sort_none,
                sort_all,
                sort_running
            };
            SSessionsSorting(const std::string& _val)
                : m_value(_val)
            {
                if (_val == "all")
                    m_typedValue = sort_all;
                else if (_val == "run")
                    m_typedValue = sort_running;
                else
                    m_typedValue = sort_none;
            }
            std::string m_value;
            ETypes m_typedValue;
        };

        /*  void validate(boost::any& _v, std::vector<std::string> const& _values, SSessionsSorting*, int)
          {
              // Make sure no previous assignment to 'v' was made.
              bpo::validators::check_first_occurrence(_v);

              // Extract the first string from 'values'. If there is more than
              // one string, it's an error, and exception will be thrown.
              const std::string& s = bpo::validators::get_single_string(_values);

              if (s == "all" || s == "run")
              {
                  _v = boost::any(SSessionsSorting(s));
              }
              else
              {
                  throw bpo::validation_error(bpo::validation_error::invalid_option_value);
              }
          }*/

        /// \brief dds-session's container of options
        typedef struct SOptions
        {
            enum ECommands
            {
                cmd_unknown,
                cmd_start,
                cmd_stop,
                cmd_stop_all,
                cmd_list,
                cmd_set_default,
                cmd_clean
            };

            SOptions()
                : m_Command(cmd_unknown)
                , m_ListSessions("")
                , m_bForce(false)
                , m_bMixed(false)
            {
            }
            static ECommands getCommandByName(const std::string& _name)
            {
                if ("start" == _name)
                    return cmd_start;
                if ("stop" == _name)
                    return cmd_stop;
                if ("stop-all" == _name)
                    return cmd_stop_all;
                if ("list" == _name)
                    return cmd_list;
                if ("set-default" == _name)
                    return cmd_set_default;
                if ("clean" == _name)
                    return cmd_clean;

                return cmd_unknown;
            }
            ECommands m_Command;
            SSessionsSorting m_ListSessions;
            bool m_bForce;
            std::string m_sSessionID;
            bool m_bMixed;
        } SOptions_t;

        // Command line parser
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options)
        {
            if (nullptr == _options)
                throw std::runtime_error("Internal error: options' container is empty.");

            // Generic options
            bpo::options_description options("dds-session options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()(
                "command",
                bpo::value<std::vector<std::string>>(),
                "The command is a name of a dds-sessions command."
                " Can be one of the following: start, stop, stop-all, list, set-default, and remove.\n\n"
                "For user's convenience it is allowed to call dds-session without \"--command\" option"
                " by just specifying the command name directly, like:\ndds-session start or dds-session stop.\n\n"
                "Commands:\n"
                "   start        : \tStart a new DDS session\n"
                "   stop         : \tStop the given DDS session\n"
                "     Argument:\n"
                "        <session id>: a session ID to stop\n"
                "   stop-all     : \tStop All running DDS sessions\n"
                "   list         : \tList DDS sessions.\n"
                "     Arguments:\n"
                "        all: list all sessions\n"
                "        run: list only running sessions\n"
                "   set-default  : \tSet a giving ID as a default DDS session\n"
                "     Argument:\n"
                "        <session id>: a session ID to set as default\n"
                "   clean        : \tRemove all STOPPED DDS sessions\n");

            options.add_options()("force,f",
                                  bpo::bool_switch(&_options->m_bForce),
                                  "Force commands without prompting for a confirmation.\n"
                                  "Can be used only with the \"remove\" command.");
            options.add_options()("mixed",
                                  bpo::bool_switch(&_options->m_bMixed),
                                  "Use worker package for a mixed environment - agents on Linux and on OS X.\n"
                                  "Can be used only with the \"start\" command.");

            //...positional
            bpo::positional_options_description pd;
            pd.add("command", 2);

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).positional(pd).run(), vm);
            bpo::notify(vm);

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
            if (vm.count("version"))
            {
                LOG(dds::misc::log_stdout) << dds::misc::DDSVersionInfoString();
                return false;
            }

            // Command
            if (vm.count("command"))
            {
                std::vector<std::string> commands = vm["command"].as<std::vector<std::string>>();
                _options->m_Command = SOptions::getCommandByName(commands[0]);
                if (SOptions::cmd_unknown == _options->m_Command)
                {
                    LOG(dds::misc::log_stderr) << "unknown command: " << commands[0] << "\n\n" << options;
                    return false;
                }

                if (SOptions::cmd_stop == _options->m_Command)
                {
                    if (commands.size() > 1)
                        _options->m_sSessionID = commands[1];
                }
                else if (SOptions::cmd_set_default == _options->m_Command)
                {
                    if (commands.size() < 2)
                    {
                        LOG(dds::misc::log_stderr)
                            << "Missing argument. The set-default command requares a sessions ID\n\n";
                        return false;
                    }
                    _options->m_sSessionID = commands[1];
                }
                else if (SOptions::cmd_list == _options->m_Command)
                {
                    if (commands.size() < 2)
                    {
                        LOG(dds::misc::log_stderr) << "Missing argument. Specify type of sorting.\n\n";
                        return false;
                    }
                    SSessionsSorting sortType(commands[1]);
                    if (SSessionsSorting::sort_none == sortType.m_typedValue)
                    {
                        LOG(dds::misc::log_stderr) << "Bad argument. Unknown type of sorting.\n\n";
                        return false;
                    }
                    _options->m_ListSessions = sortType;
                }
            }
            else
            {
                LOG(dds::misc::log_stderr) << "Nothing to do\n\n" << options;
                return false;
            }

            if (SOptions_t::cmd_set_default == _options->m_Command && _options->m_sSessionID.empty())
            {
                LOG(dds::misc::log_stderr) << "Session ID argument is missing or empty";
                return false;
            }

            return true;
        }
    } // namespace session_cmd
} // namespace dds
#endif
