// Copyright 2014 GSI, Inc. All rights reserved.
//
// TODO: Describe
//
#ifndef DDS_USERDEFAULTS_H_
#define DDS_USERDEFAULTS_H_
// BOOST
#include <boost/program_options/variables_map.hpp>
// DDS
#include "DDSOptions.h"
// STD
#include <string>

namespace dds
{
    class CUserDefaults
    {
      public:
        void init(bool _get_default = false);
        void init(const std::string& _cfgFileName, bool _get_default = false);

      public:
        std::string getValueForKey(const std::string& _Key) const;
        static std::string currentUDFile();
        static void printDefaults(std::ostream& _stream);
        static std::string getDDSPath();
        const SDDSUserDefaultsOptions_t getOptions() const
        {
            return m_options;
        }
        static std::string getServerInfoFile();

      private:
        std::string convertAnyToString(const boost::any& _any) const;
        std::string getUnifiedBoolValueForBoolKey(const std::string& _Key) const;

      private:
        boost::program_options::variables_map m_keys;
        SDDSUserDefaultsOptions_t m_options;
    };
}

#endif /* DDS_USERDEFAULTS_H_ */
