// Copyright 2014 GSI, Inc. All rights reserved.
//
// BOOST filesystem library helper
//
#ifndef BOOST_FILESYSTEM_H_
#define BOOST_FILESYSTEM_H_

// BOOST
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/format.hpp>
// MiscCommon
#include "MiscUtils.h"
// STD
#include <chrono>
#include <sstream>

namespace fs = boost::filesystem;

namespace MiscCommon
{
    namespace BOOSTHelper
    {
        inline std::string get_temp_dir(const std::string& _prefix)
        {
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            std::time_t nowTimeT = std::chrono::system_clock::to_time_t(now);
            struct std::tm* ptm = std::localtime(&nowTimeT);
            char buffer[128];
            std::strftime(buffer, 128, "%Y-%m-%d-%H-%M-%S-", ptm);
            std::chrono::milliseconds ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
            std::stringstream timeSS;
            timeSS << _prefix << "_" << buffer << boost::format("%03i") % (ms.count() % 1000);
            return timeSS.str();
        }

        /**
         *
         * @brief The function return a list of files in the deirectory with specified extension.
         * @param[in] _root - root of the directory to process.
         * @param[in] _ext - extension, for example ".log".
         * @param[out] _ret - vector of pathes to files.
         *
         */
        inline void get_files_by_extension(const fs::path& _root, const std::string& _ext, std::vector<fs::path>& _ret)
        {
            if (!fs::exists(_root))
                return;

            if (fs::is_directory(_root))
            {
                fs::recursive_directory_iterator it(_root);
                fs::recursive_directory_iterator endit;
                while (it != endit)
                {
                    if (fs::is_regular_file(*it) && it->path().extension() == _ext)
                    {
                        _ret.push_back(it->path());
                    }
                    ++it;
                }
            }
        }

        /**
         *
         * @brief The normalize_path() function removes '/' characters at the end of the of the input pathname
         * @param[in] _path - path to process.
         * @return a string, which holds the normalized path.
         *
         */
        inline std::string normalize_path(const std::string& _path)
        {
            std::string path(_path);
            MiscCommon::trim_right<std::string>(&path, '/');
            return path;
        }
        /**
         *
         * @brief The is_file() function checks whether the pathname represents a file or not.
         * @param[in] _pathname - a full path to check.
         * @return <b>true</b> if the given path represents a file and <b>false</b> otherwise.
         *
         */
        inline bool is_file(const std::string& _pathname)
        {
            bool is_valid = false;

            try
            {
                fs::path cp(normalize_path(_pathname)); //, fs::native);
                is_valid = !(fs::is_directory(cp));
            }
            catch (const fs::filesystem_error& _ex)
            {
            }

            return is_valid;
        }
        /**
         *
         * @brief the is_directory() function checks whether the pathname represents a directory or not
         * @param[in] _pathname - a path name.
         * @return <b>true</b> if the given path represents a directory and <b>false</b> otherwise.
         *
         */
        inline bool is_directory(const std::string& _pathname)
        {
            bool is_valid = false;
            try
            {
                fs::path cp(normalize_path(_pathname)); //, fs::native);
                is_valid = fs::is_directory(cp);
            }
            catch (const fs::filesystem_error& _ex)
            {
            }

            return is_valid;
        }
    }; // namespace BOOSTHelper
};     // namespace MiscCommon
#endif /*BOOST_FILESYSTEM_H_*/
