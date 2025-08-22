// Copyright 2014 GSI, Inc. All rights reserved.
//
// a set of general Command Line Interface helpers
//
#ifndef _DDS_MISCCLI_H_
#define _DDS_MISCCLI_H_

// DDS
#include "SysHelper.h"
// BOOST
#include <boost/filesystem.hpp>
#include <boost/program_options/parsers.hpp>
// STD
#include <iomanip>

namespace dds::misc
{
#if __has_include(<boost/process/v1.hpp>)
    namespace bp = boost::process::v1;
#else
    namespace bp = boost::process;
#endif

    inline void parseExe(const std::string& _exeStr,
                         const std::string& _exePrefix,
                         std::string& _filepath,
                         std::string& _filename,
                         std::string& _cmdStr)
    {
        namespace bpo = boost::program_options;
        namespace bfs = boost::filesystem;
        namespace ba = boost::algorithm;

        std::vector<std::string> split{ bpo::split_unix(_exeStr) };

        if (split.empty())
        {
            std::stringstream ss;
            ss << "Failed to parse input command line " << std::quoted(_exeStr) << ": empty output vector";
            throw std::runtime_error(ss.str());
        }

        std::string origFilepath{ split.front() };
        _filepath = origFilepath;

        smart_path(&_filepath);
        bfs::path exeFilePath{ _filepath };

        if (!exeFilePath.is_absolute() && exeFilePath.has_parent_path())
        {
            std::stringstream ss;
            ss << "Relative path " << std::quoted(_filepath)
               << " is not supported. Use either absolute path or executable name which will be searched in $PATH.";
            throw std::runtime_error(ss.str());
        }

        _filename = exeFilePath.filename().generic_string();

        // If no absolute path is given, search executable in $PATH
        if (!exeFilePath.is_absolute())
        {
            bfs::path exePath{ bp::search_path(_filename) };
            if (exePath.empty())
            {
                std::stringstream ss;
                ss << "Failed to find " << std::quoted(_filename) << " in $PATH";
                throw std::runtime_error(ss.str());
            }
            _filepath = exePath.generic_string();
        }

        // Replace original filepath with a corrected one
        _cmdStr = _exeStr;
        std::string toReplace{ (_exePrefix.empty()) ? _filepath
                                                    : (bfs::path(_exePrefix) / bfs::path(_filename)).generic_string() };
        ba::replace_first(_cmdStr, origFilepath, toReplace);
    }
}; // namespace dds::misc
#endif // _DDS_MISCCLI_H_
