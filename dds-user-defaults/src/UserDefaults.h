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

namespace DDS
{
    class CUserDefaults
    {
      public:
        void init(const std::string& _DDSCfgFileName, bool _get_default = false);
        void printDefaults(std::ostream& _stream) const;

      public:
        std::string currentUDFile() const;

      private:
        std::string convertAnyToString(const boost::any& _any) const;
        std::string getValueForKey(const std::string& _Key) const;
        std::string getUnifiedBoolValueForBoolKey(const std::string& _Key) const;
        const SDDSUserDefaultsOptions_t getOptions() const
        {
            return m_options;
        }

      private:
        boost::program_options::variables_map m_keys;
        SDDSUserDefaultsOptions_t m_options;
    };
}

#endif /* DDS_USERDEFAULTS_H_ */
