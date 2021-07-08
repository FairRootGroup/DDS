// Copyright 2014 GSI, Inc. All rights reserved.
//
// This file contains a number of helpers to calculate execution time of a function.
//
#ifndef _DDS_ENVIRONMENT_H_
#define _DDS_ENVIRONMENT_H_

// DDS
#include "BuildConstants.h"
#include "Logger.h"
// BOOST
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace dds::misc
{
    namespace impl
    {
        inline std::string getEnv(const std::string& _key)
        {
            auto value{ std::getenv(_key.c_str()) };
            return (value != nullptr) ? std::string(value) : std::string();
        }

        inline void setupEnvVar(const std::string& _location,
                                const std::string& _varName,
                                const std::string& _dirName,
                                const std::string& _defaultDir)
        {
            namespace fs = boost::filesystem;
            const fs::path locationPath(_location);
            const std::string install{ (locationPath / fs::path(_dirName)).string() };
            std::string path{ impl::getEnv(_varName) };
            // Don't prepend the same path multiple times
            if (path == install || boost::starts_with(path, install + std::string(":")))
            {
                LOG(info) << "DDS " << std::quoted(_dirName) << " directory (" << std::quoted(install)
                          << ") already prepends $" << _varName << " (" << std::quoted(path) << ")";
            }
            else
            {
                const std::string ddsInstall{ _location == kDDSInstallPrefix ? _defaultDir : install };
                path = (path.empty()) ? ddsInstall : ddsInstall + std::string(":") + path;
                setenv(_varName.c_str(), path.c_str(), 1);
                LOG(info) << "Set $" << _varName << " to " << std::quoted(path);
            }
        }

        inline void setupLocale()
        {
            // Setup locale.
            // Some Boost libraries throw an exception if the locale is not set.
            // export LC_ALL=C; unset LANGUAGE
            setenv("LC_ALL", "C", 1);
            unsetenv("LANGUAGE");
        }
    } // namespace impl

    inline void setupEnv()
    {
        impl::setupLocale();

        std::string location{ impl::getEnv("DDS_LOCATION") };

        // Setup $DDS_LOCATION
        if (location != kDDSInstallPrefix)
        {
            if (location.empty())
            {
                location = kDDSInstallPrefix;
                setenv("DDS_LOCATION", location.c_str(), 1);
                LOG(info) << "Set $DDS_LOCATION to " << std::quoted(location);
            }
            else
            {
                LOG(info) << "$DDS_LOCATION (" << std::quoted(location) << ") differs from the linked one ("
                          << std::quoted(kDDSInstallPrefix) << ")";
            }
        }

        // Setup $PATH
        impl::setupEnvVar(location, "PATH", "bin", kDDSBinaryDir);

#ifdef __APPLE__
        const std::string ldVar = "DYLD_LIBRARY_PATH";
#else
        const std::string ldVar = "LD_LIBRARY_PATH";
#endif
        // Setup $(DY)LD_LIBRARY_PATH
        impl::setupEnvVar(location, ldVar, "lib", kDDSLibraryDir);
    }
} // namespace dds::misc

#endif /*_DDS_ENVIRONMENT_H_*/
