// Copyright 2014 GSI, Inc. All rights reserved.
//
// Helps to represent data as in a HEX viewer
//
#ifndef HEXVIEW_H_
#define HEXVIEW_H_

// STD
#include <iomanip>
// MiscCommon
#include "def.h"

namespace MiscCommon
{
    /**
     * @brief This class helps to represent a given container's data as in a HEX viewer
     *
     * @note Example:
     * @code
     *
     * int main()
     {
         BYTEVector_t vec;
         vec.push_back(125);
         vec.push_back(525);
         vec.push_back(325);
         vec.push_back(123);
         vec.push_back(154);
         vec.push_back(125);
         vec.push_back(125);
         ..
         ..
         vec.push_back(167);

         cout << BYTEVectorHexView_t( vec ) << endl;
     }

     Output:
     0x00000000 | 7D 0D 45 7B 9A 7D 7D 7D 7D 7D 7D 7D 7D 7D A7 7D  | }.E{.}}}}}}}}}.}
     0x00000010 | 7D 7D 0D 45 7B 9A 7D 7D 7D 7D 7D 7D 7D 7D 7D A7  | }}.E{.}}}}}}}}}.
     0x00000020 | 7D 7D 0D 45 7B 9A 7D 7D 7D 7D 7D 7D 7D 7D 7D A7  | }}.E{.}}}}}}}}}.
     0x00000030 | 7D 7D A7                                         | }}.
     * @endcode
     *
     */
    template <class _T>
    class CHexView
    {
      public:
        CHexView(const _T& _Val, size_t _nElementsInRaw = 16)
            : m_nElementsInRaw(_nElementsInRaw)
            , m_Container(_Val)
        {
        }

        friend std::ostream& operator<<(std::ostream& _ostream, const CHexView<_T>& _this)
        {
            std::stringstream ssHex;
            ssHex << std::hex << std::uppercase;
            std::stringstream ssTxt;
            size_t nCount = 0;
            BYTEVector_t::const_iterator iter = _this.m_Container.begin();
            BYTEVector_t::const_iterator iter_end = _this.m_Container.end();
            for (; iter != iter_end; ++iter)
            {
                ssHex << std::setw(2) << std::setfill('0') << (static_cast<unsigned int>(*iter)) << ' ';
                ssTxt << (isprint(*iter) ? static_cast<char>(*iter) : '.');
                ++nCount;
                if (0 == (nCount % _this.m_nElementsInRaw))
                {
                    _this.Print(_ostream, ssHex, ssTxt, nCount);
                    ssHex.str("");
                    ssTxt.str("");
                }
            }
            if (!ssHex.str().empty())
                _this.Print(_ostream, ssHex, ssTxt, nCount);

            return _ostream;
        }

      private:
        void Print(std::ostream& _ostream,
                   const std::stringstream& _ssHex,
                   const std::stringstream& _ssTxt,
                   size_t _nCount) const
        {
            static size_t nRaw = 0;
            if (_nCount <= m_nElementsInRaw)
                nRaw = 0;
            _ostream << "0x" << std::right << std::setw(8) << std::setfill('0') << std::hex << std::uppercase
                     << (nRaw * m_nElementsInRaw) << " | " << std::left << std::setw(m_nElementsInRaw * 3)
                     << std::setfill(' ') << _ssHex.str() << " | " << std::left << std::setw(m_nElementsInRaw)
                     << _ssTxt.str() << '\n';
            ++nRaw;
        }

      private:
        const size_t m_nElementsInRaw;
        const _T& m_Container;
    };

    typedef CHexView<BYTEVector_t> BYTEVectorHexView_t;
}; // namespace MiscCommon

#endif /*HEXVIEW_H_*/
