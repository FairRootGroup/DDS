// Copyright 2014 GSI, Inc. All rights reserved.
//
// Case Insensitive String
//
#ifndef CISTRING_H_
#define CISTRING_H_

namespace MiscCommon
{
    /**
     *
     * @brief The char_traits_ci_base class is the default character traits class used for case insensitive strings.\n
     * @brief The char_traits_ci_base class is of no use by itself. It is used as a template parent class of other
     *classes, such as the char_traits_ci_t
     *template.
     * @note charT is either char or wchar_t.
     *
     */
    template <class _charT>
    struct char_traits_ci_base : std::char_traits<_charT>
    {
        static bool eq(const _charT& _Left, const _charT& _Right);
        static bool lt(const _charT& _Left, const _charT& _Right);
        static int compare(const _charT* _First1, const _charT* _First2, size_t _Count);
    };
    /**
     *
     * @brief The <b>char</b> specialization of the char_traits_ci_base class.
     *
     */
    template <>
    struct char_traits_ci_base<char> : std::char_traits<char>
    {
        typedef char _charT;
        static bool eq(const _charT& _Left, const _charT& _Right)
        {
            // test for element equality
            return (::toupper(_Left) == ::toupper(_Right));
        }
        static bool lt(const _charT& _Left, const _charT& _Right)
        {
            // test if _Left precedes _Right
            return (::toupper(_Left) < ::toupper(_Right));
        }
    };
    /**
     *
     * @brief The <b>wchar_t</b> specialization of the char_traits_ci_base class.
     *
     */
    template <>
    struct char_traits_ci_base<wchar_t> : std::char_traits<wchar_t>
    {
        typedef wchar_t _charT;
        static bool eq(const _charT& _Left, const _charT& _Right)
        {
            // test for element equality
            return (::towupper(_Left) == ::towupper(_Right));
        }
        static bool lt(const _charT& _Left, const _charT& _Right)
        {
            // test if _Left precedes _Right
            return (::towupper(_Left) < ::towupper(_Right));
        }
    };
    /**
     *
     * @brief The char_traits_ci_t class is the default character traits class used for case insensitive strings.\n
     * @brief The char_traits_ci_t class is of no use by itself. It is used as a template parameter of other classes,
     *such as the basic_string template.
     * @note charT is either char or wchar_t.
     *
     */
    template <class _charT>
    struct char_traits_ci_t : char_traits_ci_base<_charT>
    {
        typedef std::_Secure_char_traits_tag _Secure_char_traits;

        static int compare(const _charT* _First1, const _charT* _First2, size_t _Count)
        {
            // compare [_First1, _First1 + _Count) with [_First2, ...)
            return (::_memicmp(_First1, _First2, _Count * sizeof(_Elem)));
        }
        static const _charT* find(const _charT* _First, size_t _Count, const _charT& _Ch)
        {
            // look for _Ch in [_First, _First + _Count)
            for (; 0 < _Count; --_Count, ++_First)
                if (eq(*_First, _Ch))
                    return (_First);
            return (0);
        }
    };
    /**
     *
     * @brief The basic_string class is parameterized by wchar_t and by char_traits_ci_t. Represents case insensitive
     *wide string.
     *
     */
    typedef std::basic_string<wchar_t, char_traits_ci_t<wchar_t>, std::allocator<wchar_t>> ci_wstring;
    /**
     *
     * @brief The basic_string class is parameterized by char and by char_traits_ci_t. Represents case insensitive
     *string.
     *
     */
    typedef std::basic_string<char, char_traits_ci_t<char>, std::allocator<char>> ci_string;
/**
 * @brief A type definition of case insensitive string.
 * @brief It represents wide CI string if _GLIBCPP_USE_WCHAR_T and _UNICODE defined, char CI string otherwise.
 *
 */
#if defined(_GLIBCPP_USE_WCHAR_T) && defined(_UNICODE)
    typedef ci_wstring ci_tstring;
#else
    typedef ci_string ci_tstring;
#endif
}; // namespace MiscCommon

#endif /*CISTRING_H_*/
