// Copyright 2014 GSI, Inc. All rights reserved.
//
// Helpers for BOOST libraries
//
#ifndef _DDS_BOOSTHELPER_H_
#define _DDS_BOOSTHELPER_H_

// DDS
#include "MiscUtils.h"
// Boost
#include <boost/format.hpp>
#include <boost/program_options/variables_map.hpp>

namespace dds::misc
{
    inline std::string get_temp_dir(const std::string& _prefix)
    {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t nowTimeT = std::chrono::system_clock::to_time_t(now);
        struct std::tm* ptm = std::localtime(&nowTimeT);
        char buffer[128];
        std::strftime(buffer, 128, "%Y-%m-%d-%H-%M-%S-", ptm);
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        std::stringstream timeSS;
        timeSS << _prefix << "_" << buffer << boost::format("%03i") % (ms.count() % 1000);
        return timeSS.str();
    }

    /**
     *
     * @brief The conflicting_options function used to check that 'opt1' and 'opt2' are not specified at the same
     *time.
     * @param[in] _vm - a BOOST program options variable map.
     * @param[in] _opt1 - the first option to check.
     * @param[in] _opt2 - the second option to check.
     * @exception std::runtime_error - thrown if the 'opt1' and 'opt2' are specified at the same time.
     * @return no return value.
     *
     */
    inline void conflicting_options(const boost::program_options::variables_map& _vm,
                                    const char* _opt1,
                                    const char* _opt2)
    {
        if (_vm.count(_opt1) && !_vm[_opt1].defaulted() && _vm.count(_opt2) && !_vm[_opt2].defaulted())
        {
            std::string str("Command line parameter \"%1\" conflicts with \"%2\"");
            replace<std::string>(&str, "%1", _opt1);
            replace<std::string>(&str, "%2", _opt2);
            throw std::runtime_error(str);
        }
    }
    /**
     *
     * @brief The option_dependency function used to check that if 'for_what' is specified, then 'required_option'
     *is specified too.
     * @param[in] _vm - a BOOST program options variable map.
     * @param[in] _for_what - option to check.
     * @param[in] _required_option - required option.
     * @exception std::runtime_error - thrown if 'for_what' is specified but there is no 'required_option' found.
     * @return no return value.
     *
     */
    inline void option_dependency(const boost::program_options::variables_map& _vm,
                                  const char* _for_what,
                                  const char* _required_option)
    {
        if (_vm.count(_for_what) && !_vm[_for_what].defaulted() &&
            (!_vm.count(_required_option) || _vm[_required_option].defaulted()))
        {
            std::string str("Command line parameter \"%1\" must be used with \"%2\"");
            replace<std::string>(&str, "%1", _for_what);
            replace<std::string>(&str, "%2", _required_option);
            throw std::runtime_error(str);
        }
    }
}; // namespace dds::misc

#endif /*_DDS_BOOSTHELPER_H_*/
