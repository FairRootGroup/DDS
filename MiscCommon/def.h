// Copyright 2014 GSI, Inc. All rights reserved.
//
// Helpers and definitions (typedefs)
//
#ifndef DEF_H
#define DEF_H

// STD
#include <array>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
// BOOST
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#include <boost/algorithm/string/case_conv.hpp>
#pragma clang diagnostic pop
#include <boost/program_options/errors.hpp>

namespace MiscCommon
{
// Unicode support
#if defined(_GLIBCPP_USE_WCHAR_T) && defined(_UNICODE)
    /**
     *
     * @brief Long Pointer to a Constant null-Terminated String.
     * @brief It wraps \b wchar_t, when _GLIBCPP_USE_WCHAR_T and _UNICODE are
     *defined and \b char otherwise.
     *
     */
    typedef const wchar_t* LPCTSTR;
    /**
     *
     * @brief It wraps \b wchar_t, when _GLIBCPP_USE_WCHAR_T and _UNICODE are
     *defined and \b char otherwise.
     *
     */
    typedef std::basic_string<wchar_t> tstring;
    typedef std::basic_stringstream<wchar_t> tstringstream;
/**
 *
 * @brief Enclose constant strings and literal characters in the _T macro to
 *make
 * @brief them Unicode constant strings when _GLIBCPP_USE_WCHAR_T and _UNICODE
 *are defined.
 *
 */
#define _T(s) L##s
    /**
     *
     * @brief Use TCHAR instead of char or wchar_t. It will be appropriately
     *translated
     * @brief if _GLIBCPP_USE_WCHAR_T and _UNICODE are correctly defined (or
     *not).
     *
     */
    typedef wchar_t TCHAR;

#else
    /**
     *
     * @brief Long Pointer to a Constant null-Terminated String.
     * @brief It wraps \b wchar_t, when _GLIBCPP_USE_WCHAR_T and _UNICODE are
     *defined and \b char otherwise.
     *
     */
    typedef const char* LPCTSTR;
    /**
     *
     *  @brief It wraps \b wchar_t, when _GLIBCPP_USE_WCHAR_T and _UNICODE are
     *defined and \b char otherwise.
     *
     */
    typedef std::basic_string<char> tstring;
    typedef std::basic_stringstream<char> tstringstream;
/**
 *
 * @brief Use TCHAR instead of char or wchar_t. It will be appropriately
 *translated
 * @brief if _GLIBCPP_USE_WCHAR_T and _UNICODE are correctly defined (or not).
 *
 */
#define _T(s) s
    /**
     *
     * @brief Use TCHAR instead of char or wchar_t. It will be appropriately
     *translated
     * @brief if _GLIBCPP_USE_WCHAR_T and _UNICODE are correctly defined (or
     *not).
     *
     */
    typedef char TCHAR;

#endif

    /**
     *
     * @brief  A long pointer to constant string.
     *
     */
    typedef const char* LPCSTR;
    /**
     *
     * @brief An STL set of strings.
     *
     */
    typedef std::set<std::string> StringSet_t;
    /**
     *
     * @brief An STL vector of strings.
     *
     */
    typedef std::vector<std::string> StringVector_t;
    /**
     *
     * @brief An STL vector of char(s).
     *
     */
    typedef std::vector<char> CHARVector_t;
    /**
     *
     * @brief An STL vector of bytes.
     *
     */
    typedef std::vector<unsigned char> BYTEVector_t;
    /**
     *
     * @brief An STL map, which is mapping pairs of size_t (as a key) and string
     *(as a value).
     *
     */
    typedef std::map<size_t, std::string> UIntStringMap_t;
    /**
     *
     * @brief An STL map, which is mapping pairs of string (as a key) and size_t
     *(as a value)
     *
     */
    typedef std::map<std::string, size_t> StringUIntMap_t;

    /// Log Severity levels
    enum ELogSeverityLevel
    {
        proto_low,
        proto_mid,
        proto_high,
        debug,
        info,
        warning,
        error,
        fatal,
        log_stdout,
        log_stdout_clean, // nothing will be pre-append to the output
        log_stderr
    };
    const std::array<std::string, 11> g_LogSeverityLevelString{
        { "p_l", "p_m", "p_h", "dbg", "inf", "wrn", "err", "fat", "cout", "cout", "cerr" }
    };
    //=============================================================================
    // A custom streamer to help boost program options to convert string options to ELogSeverityLevel
    inline std::istream& operator>>(std::istream& _in, ELogSeverityLevel& _logSeverityLevel)
    {
        std::string token;
        _in >> token;
        boost::algorithm::to_lower(token);

        auto found = std::find(g_LogSeverityLevelString.begin(), g_LogSeverityLevelString.end(), token);
        if (found == g_LogSeverityLevelString.end())
            throw boost::program_options::invalid_option_value(token);

        _logSeverityLevel = static_cast<ELogSeverityLevel>(std::distance(g_LogSeverityLevelString.begin(), found));
        return _in;
    }

    inline std::ostream& operator<<(std::ostream& _out, ELogSeverityLevel _logSeverityLevel)
    {
        const size_t idx = static_cast<size_t>(_logSeverityLevel);
        _out << g_LogSeverityLevelString.at(idx);
        return _out;
    }
};

#endif
