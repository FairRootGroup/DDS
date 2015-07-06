// Copyright 2014 GSI, Inc. All rights reserved.
//
// TODO: Describe
//
#ifndef DDS_USERDEFAULTS_H_
#define DDS_USERDEFAULTS_H_
// BOOST
#include <boost/program_options/variables_map.hpp>
// DDS
#include "Options.h"
// STD
#include <string>

namespace dds
{
    namespace user_defaults_api
    {
        class CUserDefaults
        {
          private:
            CUserDefaults();
            ~CUserDefaults();

          public:
            // \brief Return singleton instance
            static CUserDefaults& instance();
            void reinit(const std::string& _cfgFileName, bool _get_default = false);

          private:
            void init(bool _get_default = false);
            void init(const std::string& _cfgFileName, bool _get_default = false);

          public:
            std::string getValueForKey(const std::string& _Key) const;
            static void printDefaults(std::ostream& _stream);
            const SDDSUserDefaultsOptions_t getOptions() const;
            static std::string currentUDFile();
            static std::string getDDSPath();
            std::string getServerInfoFileLocationSrv() const;
            std::string getServerInfoFileName() const;
            std::string getAgentInfoFileName() const;
            std::string getAgentInfoFileLocation() const;
            std::string getServerInfoFileLocation() const;
            std::string getWrkPkgDir() const;
            std::string getWrkPkgPath() const;
            std::string getWrkScriptPath() const;
            std::string getUserEnvScript() const;
            static std::string getAgentIDFile();
            std::string getLogFile() const;
            std::string getAgentLogStorageDir() const;
            pid_t getScoutPid() const;

          private:
            std::string convertAnyToString(const boost::any& _any) const;
            std::string getUnifiedBoolValueForBoolKey(const std::string& _Key) const;

          private:
            boost::program_options::variables_map m_keys;
            SDDSUserDefaultsOptions_t m_options;
        };
    }
}

#endif /* DDS_USERDEFAULTS_H_ */
