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
            /// \brief Return singleton instance
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
            std::string getServerInfoFileLocation() const;
            std::string getWrkPkgDir() const;
            std::string getWrkPkgPath() const;
            std::string getWrkScriptPath() const;
            std::string getUserEnvScript() const;
            static std::string getAgentIDFile();
            std::string getLogFile() const;
            std::string getAgentLogStorageDir() const;
            pid_t getScoutPid() const;
            std::string getSMInputName() const;
            std::string getSMOutputName() const;
            std::string getSMAgentInputName() const;
            std::string getSMAgentOutputName() const;
            std::string getSMAgentLeaderOutputName() const;
            std::string getPluginsRootDir() const;
            std::string getMainSIDFileName() const;
            std::string getSIDFile() const;
            std::string getSID() const;
            std::string getAgentNamedMutexName() const;

            /// \brief Returns path to the plugin's directory for specified plug-in name.
            /// \param[in] _path Path to the root plug-ins directory. If not specified (i.e. empty string is provided)
            /// than default root plug-ins directory is used.
            /// \param[in] _pluginName Name of the plug-in.
            std::string getPluginDir(const std::string& _path, const std::string& _pluginName) const;

          private:
            std::string convertAnyToString(const boost::any& _any) const;
            std::string getUnifiedBoolValueForBoolKey(const std::string& _Key) const;
            std::string getSIDName() const;

          private:
            boost::program_options::variables_map m_keys;
            SDDSUserDefaultsOptions_t m_options;
        };
    }
}

#endif /* DDS_USERDEFAULTS_H_ */
