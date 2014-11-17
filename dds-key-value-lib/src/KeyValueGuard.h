// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDS_KeyValueGuard_h
#define DDS_KeyValueGuard_h
// STD
#include <string>
// BOOST
#include <boost/property_tree/ptree.hpp>

namespace dds
{

    class CKeyValueGuard
    {
      private:
        CKeyValueGuard();
        ~CKeyValueGuard();

      public:
        static CKeyValueGuard& instance();
        void init();
        void putValue(const std::string& _key, const std::string& _value);

      private:
        const std::string getCfgFilePath() const;

      private:
        boost::property_tree::ptree m_pt;
    };
}

#endif
