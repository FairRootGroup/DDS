// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_TEST_OPTIONS_H
#define DDS_TEST_OPTIONS_H
//=============================================================================
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
// DDS
#include "ProtocolCommands.h"
#include "Res.h"
#include "version.h"
//=============================================================================
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    namespace test_cmd
    {
        /// \brief dds-getlog's container of options
        typedef struct SOptions
        {
            SOptions()
                : m_transportTest(false)
                , m_verbose(false)
            {
            }

            bool m_transportTest;
            bool m_verbose;
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
            bpo::options_description options("dds-getlog options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()("version,v", "Version information");
            options.add_options()("transport,t", "Start transport test");
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
            if (vm.count("transport"))
            {
                _options->m_transportTest = true;
            }
            if (vm.count("verbose"))
            {
                _options->m_verbose = true;
            }

            return true;
        }
    }
}
#endif
