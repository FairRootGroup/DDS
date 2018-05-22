// Copyright 2014 GSI, Inc. All rights reserved.
//
// The FindCfgFile.h header holds the CFindCfgFile class.
//
#ifndef FINDCFGFILE_H_
#define FINDCFGFILE_H_

// STD
#include <iterator>
// MiscCommon
#include "SysHelper.h"

namespace MiscCommon
{
    /**
     *
     * @brief The SFileExists functor helps to check whether the file by a given full path exists or not.
     *
     */
    template <class _T>
    struct SFileExists
    {
        bool operator()(const _T& _Path) const
        {
            _T path(_Path);
            MiscCommon::smart_path(&path);
            return (file_exists(path));
        }
    };
    /**
     *
     * @brief This class helps to find a cfg location with the best match from the given order.
     *
     * @note Example:
     * The example shows how to look for the basch cfg. file in the following order:
     * "/etc/rtt", "/etc/bashrc", "testtesttest", "$HOME/.bashrc", "/test/test/bashrc"
     * The first found cfg file will be returned by GetCfg method.
     * All environment variables in the paths will be extended.
     * @code
     *
     * int main()
     {
       CFindCfgFile<string> cfg;
       cfg.SetOrder
       ("/etc/rtt")
       ("/etc/bashrc")
       ("testtesttest")
       ("$HOME/.bashrc")
       ("/test/test/bashrc");
       string val;
       cfg.GetCfg( &val );
     }
     * @endcode
     *
     */
    template <class _T>
    class CFindCfgFile
    {
      public:
        typedef _T container_value;
        typedef std::vector<_T> container_type;

      public:
        CFindCfgFile& SetOrder(const container_value& _Path)
        {
            m_Paths.push_back(_Path);
            return *this;
        }

        CFindCfgFile& operator()(const container_value& _Path)
        {
            m_Paths.push_back(_Path);
            return *this;
        }

        void GetCfg(_T* _RetVal)
        {
            if (!_RetVal)
                return;
            typename container_type::const_iterator iter(
                find_if(m_Paths.begin(), m_Paths.end(), SFileExists<container_value>()));

            if (m_Paths.end() == iter)
                return;

            *_RetVal = *iter;
        }

        void DumpOrder(std::ostream* _stream, const container_value& _Seporator)
        {
            if (!_stream)
                return;

            std::copy(
                m_Paths.begin(), m_Paths.end(), std::ostream_iterator<container_value>(*_stream, _Seporator.c_str()));
        }

      private:
        container_type m_Paths;
    };
}; // namespace MiscCommon

#endif /*FINDCFGFILE_H_*/
