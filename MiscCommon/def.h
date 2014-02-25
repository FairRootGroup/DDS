// Copyright 2014 GSI, Inc. All rights reserved.
//
// Helpers and definitions (typedefs)
//
#ifndef DEF_H
#define DEF_H

// STD
#include <string>
#include <set>
#include <vector>
#include <map>
#include <sstream>

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
};

#endif
