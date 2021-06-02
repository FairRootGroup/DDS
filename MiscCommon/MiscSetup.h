// Copyright 2014 GSI, Inc. All rights reserved.
//
//
#ifndef _DDS_MISCSETUP_H_
#define _DDS_MISCSETUP_H_

// DDS
#include "Environment.h"
#include "Logger.h"
#include "UserDefaults.h"

namespace dds::misc
{
    template <class Options_t>
    inline int defaultExecSetup(int _argc,
                                char* _argv[],
                                Options_t* _options,
                                std::function<bool(int argc, char* argv[], Options_t*)> _parseFunc)
    {
        try
        {
            user_defaults_api::CUserDefaults::instance(); // Initialize user defaults
            MiscCommon::Logger::instance().init();        // Initialize log
            dds::misc::setupEnv();                        // Setup environment

            std::vector<std::string> arguments(_argv + 1, _argv + _argc);
            std::ostringstream ss;
            std::copy(arguments.begin(), arguments.end(), std::ostream_iterator<std::string>(ss, " "));
            LOG(MiscCommon::info) << "Starting with arguments: " << ss.str();

            if (!_parseFunc(_argc, _argv, _options))
                return EXIT_FAILURE;
        }
        catch (std::exception& e)
        {
            LOG(MiscCommon::log_stderr) << e.what();
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    inline int defaultExecReinit(const boost::uuids::uuid& _sid)
    {
        try
        {
            user_defaults_api::CUserDefaults::instance().reinit(
                _sid, user_defaults_api::CUserDefaults::instance().currentUDFile());
            MiscCommon::Logger::instance().reinit();
        }
        catch (std::exception& e)
        {
            LOG(MiscCommon::log_stderr) << e.what();
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
} // namespace dds::misc

#endif /*_DDS_MISCSETUP_H_*/
