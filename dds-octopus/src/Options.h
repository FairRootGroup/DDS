// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_OCTOPUS_OPTIONS_H
#define DDS_OCTOPUS_OPTIONS_H
//=============================================================================
// STD
#include <iostream>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// DDS
#include "version.h"
//=============================================================================
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    namespace dds_octopus
    {
        typedef struct SOptions
        {
            SOptions()
                : m_taskCount(0)
            {
            }

            size_t m_taskCount;
        } SOptions_t;
        //=============================================================================
        inline void PrintVersion()
        {
            std::cout << " v" << PROJECT_VERSION_STRING << "\n"
                      << "DDS configuration"
                      << " v" << USER_DEFAULTS_CFG_VERSION << std::endl;
        }
        //=============================================================================
        // Command line parser
        inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options) throw(std::exception)
        {
            if (nullptr == _options)
                throw std::runtime_error("Internal error: options' container is empty.");

            // Generic options
            bpo::options_description options("dds-octopus options");
            options.add_options()("help,h", "Produce help message");
            options.add_options()(
                "number,n", bpo::value<size_t>(&_options->m_taskCount)->default_value(0), "Task count");

            // Parsing command-line
            bpo::variables_map vm;
            bpo::store(bpo::command_line_parser(_argc, _argv).options(options).run(), vm);
            bpo::notify(vm);

            if (vm.count("help") || vm.empty())
            {
                std::cout << options << std::endl;
                return false;
            }

            return true;
        }
    }
}
#endif
