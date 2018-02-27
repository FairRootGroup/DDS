// Copyright 2014 GSI, Inc. All rights reserved.
//
// Helpers for BOOST libraries
//
#ifndef BOOSTHELPER_H_
#define BOOSTHELPER_H_

// BOOST
// FIX: silence a warning until BOOST fix it
// boost/thread/pthread/condition_variable.hpp:53:19: warning: unused variable 'res' [-Wunused-variable]
// clang
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif
// gcc
#if defined(__GNUG__) && (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
// push doesn't work for gcc 4.2
//#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#include <boost/thread/thread.hpp>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#if defined(__GNUG__) && (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
//#pragma GCC diagnostic pop
#pragma GCC diagnostic warning "-Wunused-variable"
#endif

#include <boost/program_options/variables_map.hpp>
#include <boost/shared_ptr.hpp>
// MiscCommon
#include "MiscUtils.h"

namespace MiscCommon
{
    /**
     *
     * @brief a BOOST helpers namespace
     *
     */
    namespace BOOSTHelper
    {
        /**
         *
         * @brief A smart pointer wrapper for boost::thread pointers.
         *
         */
        typedef boost::shared_ptr<boost::thread> Thread_PTR_t;
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
                MiscCommon::replace<std::string>(&str, "%1", _opt1);
                MiscCommon::replace<std::string>(&str, "%2", _opt2);
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
                MiscCommon::replace<std::string>(&str, "%1", _for_what);
                MiscCommon::replace<std::string>(&str, "%2", _required_option);
                throw std::runtime_error(str);
            }
        }
    };
};

#endif /*BOOSTHELPER_H_*/
