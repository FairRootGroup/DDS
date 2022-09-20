// Copyright 2014 GSI, Inc. All rights reserved.
//
// a set of general helpers
//
#ifndef _DDS_MISCUTILS_H_
#define _DDS_MISCUTILS_H_

// STD
#include <algorithm>
#include <iostream>
// BOOST
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/program_options/parsers.hpp>

/**
 *
 *  @brief Miscellaneous functions and helpers are located here
 *
 */
namespace dds::misc
{
     // Workaround: Since std::unary_function is removed in C++17, we provide a simple workaround for the old code until it is upgraded to modern new C++
    template <class ArgumentType, class ResultType>
    struct workaround_unary_function
    {
        typedef ArgumentType argument_type;
        typedef ResultType result_type;
    };

    template <class Argument1Type, class Argument2Type, class ResultType>
    struct workaround_binary_function
    {
        typedef Argument1Type first_argument_type;
        typedef Argument2Type second_argument_type;
        typedef ResultType result_type;
    };

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

    struct HumanReadable
    {
        std::uintmax_t size{};

      private:
        friend std::ostream& operator<<(std::ostream& os, HumanReadable hr)
        {
            int i{};
            double mantissa = hr.size;
            for (; mantissa >= 1024.; mantissa /= 1024., ++i)
            {
            }
            mantissa = std::ceil(mantissa * 10.) / 10.;
            os << mantissa << "BKMGTPE"[i];
            return i == 0 ? os : os << "B (" << hr.size << ')';
        }
    };
};     // namespace dds::misc
#endif // _DDS_MISCUTILS_H_
