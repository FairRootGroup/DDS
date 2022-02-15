// Copyright 2014 GSI, Inc. All rights reserved.
//
// TODO: Describe
//
#ifndef DDS_USERDEFAULTS_H_
#define DDS_USERDEFAULTS_H_
// BOOST
#include <boost/program_options/variables_map.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
// DDS
#include "Options.h"
// STD
#include <string>
#include <vector>

namespace dds
{
    namespace user_defaults_api
    {
        class CUserDefaults
        {
          private:
            CUserDefaults(const boost::uuids::uuid& _sid);
            ~CUserDefaults();
            CUserDefaults(const CUserDefaults&) = delete;
            CUserDefaults(CUserDefaults&&) = delete;
            CUserDefaults& operator=(const CUserDefaults&) = delete;
            CUserDefaults& operator=(CUserDefaults&&) = delete;

          public:
            /// \brief Return singleton instance
            static CUserDefaults& instance(const boost::uuids::uuid& _sid = CUserDefaults::getInitialSID());
            void reinit(const boost::uuids::uuid& _sid, const std::string& _cfgFileName, bool _get_default = false);

          public:
            static boost::uuids::uuid getInitialSID();

          private:
            void init(bool _get_default = false);
            void init(const std::string& _cfgFileName, bool _get_default = false);
            void makeDefaultDirs();

          public:
            std::string getValueForKey(const std::string& _key) const;
            static void printDefaults(std::ostream& _stream);
            const SDDSUserDefaultsOptions_t getOptions() const;
            static std::string currentUDFile();
            static std::string getDDSPath();
            std::string getWrkDir() const;
            std::string getServerInfoFileLocationSrv() const;
            std::string getServerInfoFileName() const;
            std::string getServerInfoFileLocation() const;
            std::string getWrkPkgRootDir() const;
            std::string getWrkPkgDir(const std::string& _SubmissionID) const;
            std::string getWrkPkgPath(const std::string& _SubmissionID) const;
            std::string getWrkScriptPath(const std::string& _SubmissionID) const;
            std::string getUserEnvScript() const;
            static std::string getAgentIDFilePath();
            static std::string getAgentIDFileName();
            std::string getLogFile() const;
            std::string getAgentLogStorageDir() const;
            pid_t getScoutPid() const;
            std::string getSMLeaderOutputName(uint64_t _protocolHeaderID) const;
            std::string getSMLeaderInputName(uint64_t _protocolHeaderID) const;
            std::vector<std::string> getSMLeaderInputNames() const;
            std::string getPluginsRootDir() const;
            std::string getMainSIDFile() const;
            std::string getSIDFile() const;
            std::string getLockedSID() const;
            std::string getCurrentSID() const;
            std::string getDefaultSIDLinkName() const;
            std::string getDefaultSID() const;
            void setDefaultSID(const boost::uuids::uuid& _sid) const noexcept;
            std::string getSessionsRootDir() const;
            std::string getSessionsHolderDirName() const;
            bool IsSessionRunning(const boost::uuids::uuid& _sid = boost::uuids::nil_uuid()) const;
            std::string getCommanderPidFileName() const;
            std::string getCommanderPidFile(const boost::uuids::uuid& _sid = boost::uuids::nil_uuid()) const;
            std::string getWnBinsDir() const;
            static std::string getTopologyXSDFilePath();
            bool isAgentInstance() const;
            static size_t getNumLeaderFW(); // Number of SM agent outputs
            std::string getSlotsRootDir() const;

            /// \brief Returns path to the plugin's directory for specified plug-in name.
            /// \param[in] _path Path to the root plug-ins directory. If not specified (i.e. empty string is provided)
            /// than default root plug-ins directory is used.
            /// \param[in] _pluginName Name of the plug-in.
            std::string getPluginDir(const std::string& _path, const std::string& _pluginName) const;

          private:
            std::string convertAnyToString(const boost::any& _any) const;
            std::string getUnifiedBoolValueForBoolKey(const std::string& _Key) const;
            std::string getSIDName() const;
            void addSessionIDtoPath(std::string& _path) const;
            void addSessionIDtoPath(std::string& _path, const boost::uuids::uuid& _sid_) const;
            void setSessionID(const boost::uuids::uuid& _sid);

          private:
            boost::program_options::variables_map m_keys;
            SDDSUserDefaultsOptions_t m_options;
            std::string m_sessionID;
        };
    } // namespace user_defaults_api
} // namespace dds

#endif /* DDS_USERDEFAULTS_H_ */
