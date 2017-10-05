// Copyright 2017 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__SESSIONIDFILE__
#define __DDS__SESSIONIDFILE__
// BOOST
#include <boost/filesystem.hpp>

namespace MiscCommon
{
    class CSessionIDFile
    {
      public:
        CSessionIDFile(const std::string& _sidFile)
            : m_pathSIDFile(_sidFile)
            , m_bHasCreated(false)
        {
        }
        ~CSessionIDFile()
        {
            if (m_bHasCreated)
                remove();
        }

        void create()
        {
            remove();
            m_bHasCreated = true;

            boost::uuids::uuid sid = boost::uuids::random_generator()();
            std::ofstream f(m_pathSIDFile.string());
            f << sid;
            f.close();
        }

        std::string getSID()
        {
            if (!boost::filesystem::is_regular_file(m_pathSIDFile))
                return std::string();
            std::string sid;
            std::ifstream f(m_pathSIDFile.string());
            f >> sid;

            return sid;
        }

      private:
        void remove()
        {
            if (boost::filesystem::is_regular_file(m_pathSIDFile))
                boost::filesystem::remove(m_pathSIDFile);
        }

      private:
        boost::filesystem::path m_pathSIDFile;
        bool m_bHasCreated;
    };
}
#endif /* defined(__DDS__SESSIONIDFILE__) */
