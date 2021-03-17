// Copyright 2014 GSI, Inc. All rights reserved.
//
// a set of general helpers
//
#ifndef MISCUTILS_H
#define MISCUTILS_H

#include "wordexp.h"
// STD
#include <algorithm>
#include <iostream>
// BOOST
#include <boost/filesystem.hpp>
#include <boost/process.hpp>

/**
 *
 *  @brief Miscellaneous functions and helpers are located here
 *
 */
namespace MiscCommon
{
    /**
     *
     * @brief Class which makes child to be non-copyable object.
     *
     */
    class NONCopyable
    {
      protected:
        NONCopyable()
        {
        }
        ~NONCopyable()
        {
        }

      private:
        NONCopyable(const NONCopyable&);
        const NONCopyable& operator=(const NONCopyable&);
    };

    class NullType
    {
    };
    /**
     *
     * @brief A helper class. Helps to automatically track environment variables.
     *
     */
    class auto_setenv
    {
      public:
        auto_setenv()
            : m_unset(false)
        {
        }
        auto_setenv(const std::string& _VarName, const std::string& _NewValue)
            : m_unset(false)
        {
            m_sVarName = _VarName;
            m_sNewValue = _NewValue;

            char* chTmp = getenv(m_sVarName.c_str());
            if (chTmp)
                m_sOldValue = chTmp;
            else
                m_unset = true;
            // TODO: check error code
            setenv(m_sVarName.c_str(), m_sNewValue.c_str(), 1);
        }
        ~auto_setenv()
        {
            unset();
        }

      public:
        void set(const std::string& _VarName, const std::string& _NewValue)
        {
            unset();

            m_sVarName = _VarName;
            m_sNewValue = _NewValue;

            char* chTmp = getenv(m_sVarName.c_str());
            if (chTmp)
                m_sOldValue = chTmp;
            else
                m_unset = true;
            // TODO: check error code
            setenv(m_sVarName.c_str(), m_sNewValue.c_str(), 1);
        }
        void unset()
        {
            if (m_unset)
            {
                m_unset = false;
                unsetenv(m_sVarName.c_str());
                return;
            }

            if (!m_sVarName.empty())
            {
                m_sVarName.clear();
                setenv(m_sVarName.c_str(), m_sOldValue.c_str(), 1);
            }
        }

      private:
        std::string m_sVarName;
        std::string m_sNewValue;
        std::string m_sOldValue;
        bool m_unset;
    };

    /**
     *
     *  @brief appends character _ItemToAdd to the string _pString if there is no such suffix on the end of _pString.
     *  @param[in] _pString - The string to be processed.
     *  @param[in] _ItemToAdd - The target characters to be checked and added.
     *  @return A pointer to the processed string object.
     *
     */
    template <typename _T>
    _T* smart_append(_T* _pString, const typename _T::value_type _ItemToAdd)
    {
        if (!_pString)
            return _pString;

        if (_pString->empty() || (*_pString)[_pString->size() - 1] != _ItemToAdd)
            _pString->push_back(_ItemToAdd);

        return _pString;
    }

    /**
     *
     *  @brief trims trailing characters from the string.
     *  @param[in] _pString - The string to be trimmed.
     *  @param[in] _chWhat - The target character to be trimmed.
     *  @return A reference to the string object from which the elements have been trimmed.
     *
     */
    template <typename _T>
    _T& trim_right(_T* _pString, const typename _T::value_type& _chWhat)
    {
        return _pString->erase(_pString->find_last_not_of(_chWhat) + 1);
    }

    /**
     *
     *  @brief trims leading characters from the string.
     *  @param[in] _pString - The string to be trimmed.
     *  @param[in] _chWhat - The target character to be trimmed.
     *  @return A reference to the string object from which the elements have been trimmed.
     *
     */
    template <typename _T>
    _T& trim_left(_T* _pString, const typename _T::value_type& _chWhat)
    {
        return _pString->erase(0, _pString->find_first_not_of(_chWhat));
    }

    /**
     *
     *  @brief trims trailing and leading characters from the string.
     *  @param[in] _pString - The string to be trimmed.
     *  @param[in] _chWhat - The target character to be trimmed.
     *  @return A reference to the string object from which the elements have been trimmed.
     *
     */
    template <typename _T>
    _T& trim(_T* _pString, const typename _T::value_type& _chWhat)
    {
        return trim_right(&trim_left(_pString, _chWhat), _chWhat);
    }

    /**
     *
     *  @brief finds elements in a string match a specified string and replaces it.
     *  @param[in,out] _pString - The string to be processed.
     *  @param[in] _what - String to be replaced.
     *  @param[in] _with - Replacing string.
     *  @return A reference to the string object from which the elements have been replaced.
     *
     */
    template <typename _T>
    _T& replace(_T* _pString, const _T& _what, const _T& _with)
    {
        typename _T::size_type pos = 0;
        typename _T::size_type withLen = _with.length();
        typename _T::size_type whatLen = _what.length();
        while ((pos = _pString->find(_what, pos)) != _T::npos)
        {
            _pString->replace(pos, _what.length(), _with);
            if (withLen > whatLen)
                pos += withLen - whatLen + 1;
        }
        return (*_pString);
    }

    // HACK: because of the bug in gcc 3.3 we need to use this nasty ToLower and ToUpper instead of direct calls of
    // tolower (tolower.. is inline in this version
    // of std lib)...
    struct ToLower
    {
        char operator()(char c) const
        {
            return std::tolower(c);
        }
    };
    struct ToUpper
    {
        char operator()(char c) const
        {
            return std::toupper(c);
        }
    };
    struct IsDigit : std::unary_function<int, int>
    {
        int operator()(int c) const
        {
            return std::isdigit(c);
        }
    };

    /**
     *
     *  @brief convert string to upper case.
     *  @param[in] _str - Sting to convert.
     *  @return Converted string.
     *
     */
    template <typename _T>
    _T& to_upper(_T& _str)
    {
        std::transform(_str.begin(), _str.end(), _str.begin(), ToUpper());
        return _str;
    }

    /**
     *
     *  @brief convert string to lower case.
     *  @param[in] _str - Sting to convert.
     *  @return Converted string.
     *
     */
    template <typename _T>
    _T& to_lower(_T& _str)
    {
        std::transform(_str.begin(), _str.end(), _str.begin(), ToLower());
        return _str;
    }

    inline void parseExe(const std::string& _exeStr,
                         const std::string& _exePrefix,
                         std::string& _filePath,
                         std::string& _filename,
                         std::string& _cmdStr)
    {
        // wordexp will always fail with WRDE_SYNTAX if you have set the SIGCHLD signal to be ignored like so:
        // signal(SIGCHLD, SIG_IGN). A library may be doing this without your knowledge. Presumably the implementation
        // of wordexp on OS X actually spawns a shell as a child process to do the parsing. The solution is to call
        // signal(SIGCHLD, SIG_DFL) before wordexp. You can restore signal(SIGCHLD, SIG_IGN) afterward.
        boost::process::posix::sighandler_t old_sig = signal(SIGCHLD, SIG_IGN);
        signal(SIGCHLD, SIG_DFL);

        // Expand the string for the program to extract exe name and command line arguments
        wordexp_t result;
        int err = wordexp(_exeStr.c_str(), &result, 0);

        // restore old signal
        signal(SIGCHLD, old_sig);

        switch (err)
        {
            case 0:
            {
                _filePath = result.we_wordv[0];

                boost::filesystem::path exeFilePath(_filePath);

                if (!exeFilePath.is_absolute() && exeFilePath.has_parent_path())
                    throw std::runtime_error(
                        "Relative paths are not supported: " + _filePath +
                        ". Use either absolute path or executable name which will be searched in PATH.");

                _filename = exeFilePath.filename().generic_string();

                // If no absolute path is given, search executable in PATH
                if (!exeFilePath.is_absolute())
                {
                    boost::filesystem::path exePath = boost::process::search_path(_filename);
                    _filePath = exePath.generic_string();
                }

                _cmdStr = (_exePrefix.empty()) ? _filePath : (_exePrefix + _filename);
                for (size_t i = 1; i < result.we_wordc; ++i)
                {
                    _cmdStr += " \"";
                    _cmdStr += result.we_wordv[i];
                    _cmdStr += "\"";
                }

                wordfree(&result);
            }
            break;
            case WRDE_NOSPACE:
                // If the error was WRDE_NOSPACE,
                // then perhaps part of the result was allocated.
                throw std::runtime_error("memory error occurred while processing the user's executable path: " +
                                         _exeStr);
                break;

            case WRDE_BADCHAR:
                throw std::runtime_error("Illegal occurrence of newline or one of |, &, ;, <, >, (, ), {, } in " +
                                         _exeStr);
                break;

            case WRDE_BADVAL:
                throw std::runtime_error(
                    "An undefined shell variable was referenced, and the WRDE_UNDEF flag told us to "
                    "consider this an error in " +
                    _exeStr);
                break;

            case WRDE_CMDSUB:
                throw std::runtime_error(
                    "Command substitution occurred, and the WRDE_NOCMD flag told us to consider this an error in " +
                    _exeStr);
                break;
            case WRDE_SYNTAX:
                throw std::runtime_error("Shell syntax error, such as unbalanced parentheses or unmatched quotes in " +
                                         _exeStr);
                break;

            default: // Some other error.
                throw std::runtime_error("failed to process the user's executable path: " + _exeStr);
        }
    }
}; // namespace MiscCommon
#endif
