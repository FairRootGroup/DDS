// Copyright 2017 GSI, Inc. All rights reserved.
//
//
//
#ifndef _DDS_SESSIONIDFILE_
#define _DDS_SESSIONIDFILE_
// BOOST
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace dds::misc
{
    class CSessionIDFile
    {
      public:
        CSessionIDFile()
            : m_bLocked(false)
        {
        }
        CSessionIDFile(const std::string& _sidFile)
            : m_pathSIDFile(_sidFile)
            , m_bLocked(false)
            , m_sid(boost::uuids::nil_uuid())
        {
        }
        ~CSessionIDFile()
        {
            if (m_bLocked)
                unlock();
        }

        boost::uuids::uuid generate()
        {
            m_sid = boost::uuids::random_generator()();
            return m_sid;
        }

        std::string string()
        {
            return boost::lexical_cast<std::string>(m_sid);
        }

        void lock(const boost::uuids::uuid& _sid, const std::string& _sidFile = "")
        {
            if (!_sidFile.empty())
                m_pathSIDFile = _sidFile;

            std::ofstream f(m_pathSIDFile.string());
            f << _sid;
            f.close();
            m_bLocked = true;
        }

        void unlock()
        {
            if (boost::filesystem::is_regular_file(m_pathSIDFile))
                boost::filesystem::remove(m_pathSIDFile);

            m_bLocked = false;
        }

        std::string getLockedSID()
        {
            if (!boost::filesystem::is_regular_file(m_pathSIDFile))
                return std::string();
            std::string sid;
            std::ifstream f(m_pathSIDFile.string());
            f >> sid;

            return sid;
        }

      private:
        boost::filesystem::path m_pathSIDFile;
        bool m_bLocked;
        boost::uuids::uuid m_sid;
    };
} // namespace dds::misc
#endif /* defined(_DDS_SESSIONIDFILE_) */
