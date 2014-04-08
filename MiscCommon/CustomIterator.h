// Copyright 2014 GSI, Inc. All rights reserved.
//
// Custom extensions to STL
//
#ifndef CUSTOMITERATOR_H_
#define CUSTOMITERATOR_H_

namespace MiscCommon
{
    /**
     *
     * @brief This custom istream iterator helps to read input line by line without breaking lines after whitespace etc.
     * @note Usage:
     * @code
        ifstream f("test.txt");
        std::vector<std::string> vec;
        std::copy(custom_istream_iterator<std::string>(f),
                      custom_istream_iterator<std::string>(),
                      std::back_inserter(vec));
     * @endcode
     *
     */
    template <class T, class Ch = char, class Tr = std::char_traits<Ch>, class Dist = std::ptrdiff_t>
    class custom_istream_iterator : public std::iterator<std::input_iterator_tag, T, Dist, const T*, const T&>
    {
      public:
        typedef custom_istream_iterator<T, Ch, Tr, Dist> m_it;
        typedef Ch char_type;
        typedef Tr traits_type;
        typedef std::basic_istream<Ch, Tr> istream_type;

        // construct singular iterator
        custom_istream_iterator()
            : m_istream(0)
        {
        }

        // construct with input stream
        custom_istream_iterator(istream_type& s)
            : m_istream(&s)
        {
            getval();
        }

        // return designated value
        const T& operator*() const
        {
            return m_val;
        }

        // return pointer to class object
        const T* operator->() const
        {
            return &**this;
        }

        // preincrement
        custom_istream_iterator& operator++()
        {
            getval();
            return *this;
        }

        // postincrement
        custom_istream_iterator operator++(int)
        {
            m_it tmp = *this;
            ++*this;
            return tmp;
        }

        // test for iterator equality
        bool equal(const m_it& rhs) const
        {
            return m_istream == rhs.m_istream;
        }

      protected:
        // get a T value if possible
        void getval()
        {
            if (m_istream != 0 && !(*m_istream >> m_val))
                m_istream = 0;
        }

        istream_type* m_istream; // pointer to input stream
        T m_val;                 // lookahead value (valid if m_istream is not null)
    };

    // specialization for std::string
    template <>
    inline void custom_istream_iterator<std::basic_string<char>, char, std::char_traits<char>, std::ptrdiff_t>::getval()
    {
        if (m_istream != 0 && !(std::getline(*m_istream, m_val)))
            m_istream = 0;
    }

    // m_istream_iterator template operators
    // test for m_istream_iterator equality
    template <class T, class Ch, class Tr, class Dist>
    inline bool operator==(const custom_istream_iterator<T, Ch, Tr, Dist>& lhs, const custom_istream_iterator<T, Ch, Tr, Dist>& rhs)
    {
        return lhs.equal(rhs);
    }

    template <class T, class Ch, class Tr, class Dist>
    inline bool operator!=(const custom_istream_iterator<T, Ch, Tr, Dist>& lhs, const custom_istream_iterator<T, Ch, Tr, Dist>& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /*CUSTOMITERATOR_H_*/
