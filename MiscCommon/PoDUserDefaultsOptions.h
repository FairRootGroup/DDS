// Copyright 2014 GSI, Inc. All rights reserved.
//
// IXMLPersist is a persistence interface.
//
/************************************************************************/
/**
 * @file PoDUserDefaultsOptions.h
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-06-30
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009-2012 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PODUSERDEFAULTSOPTIONS_H_
#define PODUSERDEFAULTSOPTIONS_H_
// STD
#include <fstream>
// BOOST
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#if defined(BOOST_PROPERTY_TREE)
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#endif
// MiscCommon
#include "FindCfgFile.h"

namespace PoD
{
    class CPoDUserDefaults;

    inline std::string showCurrentPUDFile()
    {
        MiscCommon::CFindCfgFile<std::string> cfg;
        cfg.SetOrder("$HOME/.PoD/PoD.cfg")("$POD_LOCATION/etc/PoD.cfg")("$POD_LOCATION/PoD.cfg");
        std::string val;
        cfg.GetCfg(&val);

        return val;
    }

    typedef struct SCommonOptions
    {
        std::string m_workDir;      //!< Working folder.
        std::string m_logFileDir;   //!< The log filename.
        bool m_logFileOverwrite;    //!< Overwrite log file each session.
        int m_shutdownIfIdleForSec; //!< Shut down agent if its idle time is higher this value. If value is 0 then the feature is off.
        unsigned short m_logLevel;  //!< A log level
        unsigned int m_xproofPortsRangeMin;
        unsigned int m_xproofPortsRangeMax;
        unsigned int m_agentNodeReadBuffer; //!< A buffer size, used by a proxy (in bytes).
    } SCommonOptions_t;

    typedef struct SServerOptions
    {
        //
        // ---= SERVER =---
        //
        SCommonOptions_t m_common;
        // SahredFS is used as a location for job scripts.
        // Some RMSs require that job scripts reside on FS which is accessible by its WNs.
        // We can't use m_workDir for this, because it could be a case that the only shared FS is AFS, which
        // can't be used as PoD's working director as it doesn't support pipes.
        std::string m_sandboxDir;
        unsigned int m_agentLocalClientPortMin;
        unsigned int m_agentLocalClientPortMax;
        unsigned int m_agentPortsRangeMin;
        unsigned int m_agentPortsRangeMax;
        std::string m_packetForwarding;
        unsigned int m_agentThreads; //!< A number of threads in thread pool.
        std::string m_proofCfgEntryPattern;
        bool m_genTempSSHKeys; //!< Indicates, whether PoD need to generate temporary SSH keys for WNs. Useful in case PoD server has only ssh port open for
                               //incoming connection.
    } SServerOptions_t;

    typedef struct SWorkerOptions
    {
        //
        // ---= WORKER =---
        //
        SCommonOptions_t m_common;
        bool m_setMyROOTSYS;     //!< Whether to use user's ROOTSYS to use on workers (values: yes/no)
        std::string m_myROOTSYS; //!< User's ROOTSYS to use on workers
    } SWorkerOptions_t;

    typedef struct SLSFOptions
    {
        //
        // ---= LSF =---
        //
        bool m_emailOutput;  //!< specifies whether job's output is sent to the user by mail
        bool m_uploadJobLog; //!< specifies whether to upload jobs log files from workers when PoD jobs are completed.
    } SLSFOptions_t;

    typedef struct SPBSOptions
    {
        //
        // ---= PBS =---
        //
        bool m_uploadJobLog;       //!< specifies whether to upload jobs log files from workers when PoD jobs are completed.
        std::string m_optionsFile; //!< a ful path to the file, which contains additional PBS options
    } SPBSOptions_t;

    typedef struct SOGEOptions
    {
        //
        // ---= OGE/SGE: Grid Engine =---
        //
        bool m_uploadJobLog;       //!< specifies whether to upload jobs log files from workers when PoD jobs are completed.
        std::string m_optionsFile; //!< a ful path to the file, which contains GE options
    } SOGEOptions_t;

    typedef struct SCondorOptions
    {
        //
        // ---= Condor =---
        //
        bool m_uploadJobLog;       //!< specifies whether to upload jobs log files from workers when PoD jobs are completed.
        std::string m_optionsFile; //!< a ful path to the file, which contains additional job desciption options (Condor's jdl)
    } SCondorOptions_t;

    typedef struct SSlurmOptions
    {
        //
        // ---= SLURM =---
        //
        bool m_uploadJobLog; //!< specifies whether to upload jobs log files from workers when PoD jobs are completed.
    } SSlurmOptions_t;

    typedef struct SPoDUserDefaultOptions
    {
        SServerOptions_t m_server;
        SWorkerOptions_t m_worker;
        SLSFOptions_t m_lsf;
        SPBSOptions_t m_pbs;
        SOGEOptions_t m_oge;
        SCondorOptions_t m_condor;
        SSlurmOptions_t m_slurm;
    } SPoDUserDefaultsOptions_t;

    // TODO: we use boost 1.32. This is the only method I found to convert boost::any to string.
    // In the next version of boost its solved.
    inline std::string convertAnyToString(const boost::any& _any)
    {
        if (_any.type() == typeid(std::string))
            return boost::any_cast<std::string>(_any);

        std::ostringstream ss;
        if (_any.type() == typeid(int))
            ss << boost::any_cast<int>(_any);

        if (_any.type() == typeid(unsigned int))
            ss << boost::any_cast<unsigned int>(_any);

        if (_any.type() == typeid(unsigned char))
            ss << boost::any_cast<unsigned char>(_any);

        if (_any.type() == typeid(unsigned short))
            ss << boost::any_cast<unsigned short>(_any);

        if (_any.type() == typeid(bool))
            ss << boost::any_cast<bool>(_any);

        return ss.str();
    }

    class CPoDUserDefaults
    {
    public:
        void init(const std::string& _PoDCfgFileName, bool _get_default = false)
        {
            m_keys.clear();
            boost::program_options::options_description config_file_options("PoD user defaults options");
            // HACK: Don't make a long add_options, otherwise Eclipse 3.5's CDT indexer can't handle it
            config_file_options.add_options()(
                "server.work_dir", boost::program_options::value<std::string>(&m_options.m_server.m_common.m_workDir)->default_value("$HOME/.PoD"), "")(
                "server.sandbox_dir", boost::program_options::value<std::string>(&m_options.m_server.m_sandboxDir)->default_value("$HOME/.PoD"), "")(
                "server.logfile_dir",
                boost::program_options::value<std::string>(&m_options.m_server.m_common.m_logFileDir)->default_value("$HOME/.PoD/log"),
                "")("server.logfile_overwrite",
                    boost::program_options::value<bool>(&m_options.m_server.m_common.m_logFileOverwrite)->default_value(true, "yes"),
                    "")("server.log_level", boost::program_options::value<unsigned short>(&m_options.m_server.m_common.m_logLevel)->default_value(1), "")(
                "server.agent_shutdown_if_idle_for_sec",
                boost::program_options::value<int>(&m_options.m_server.m_common.m_shutdownIfIdleForSec)->default_value(1800),
                "")("server.agent_local_client_port_min",
                    boost::program_options::value<unsigned int>(&m_options.m_server.m_agentLocalClientPortMin)->default_value(20000),
                    "")("server.agent_local_client_port_max",
                        boost::program_options::value<unsigned int>(&m_options.m_server.m_agentLocalClientPortMax)->default_value(25000),
                        "")("server.xproof_ports_range_min",
                            boost::program_options::value<unsigned int>(&m_options.m_server.m_common.m_xproofPortsRangeMin)->default_value(21001))(
                "server.xproof_ports_range_max",
                boost::program_options::value<unsigned int>(&m_options.m_server.m_common.m_xproofPortsRangeMax)->default_value(22000))(
                "server.agent_ports_range_min", boost::program_options::value<unsigned int>(&m_options.m_server.m_agentPortsRangeMin)->default_value(22001))(
                "server.agent_ports_range_max", boost::program_options::value<unsigned int>(&m_options.m_server.m_agentPortsRangeMax)->default_value(23000))(
                "server.agent_threads", boost::program_options::value<unsigned int>(&m_options.m_server.m_agentThreads)->default_value(5))(
                "server.agent_node_readbuffer",
                boost::program_options::value<unsigned int>(&m_options.m_server.m_common.m_agentNodeReadBuffer)->default_value(5000))(
                "server.packet_forwarding", boost::program_options::value<std::string>(&m_options.m_server.m_packetForwarding)->default_value("auto"), "")(
                "server.proof_cfg_entry_pattern",
                boost::program_options::value<std::string>(&m_options.m_server.m_proofCfgEntryPattern)
                    ->default_value("worker %user%@%host% port=%port% pref=100"),
                "")("server.gen_temp_ssh_keys", boost::program_options::value<bool>(&m_options.m_server.m_genTempSSHKeys)->default_value(false), "");
            config_file_options.add_options()(
                "worker.work_dir", boost::program_options::value<std::string>(&m_options.m_worker.m_common.m_workDir)->default_value("$POD_LOCATION/"), "")(
                "worker.logfile_dir",
                boost::program_options::value<std::string>(&m_options.m_worker.m_common.m_logFileDir)->default_value("$POD_LOCATION/"),
                "")("worker.logfile_overwrite",
                    boost::program_options::value<bool>(&m_options.m_worker.m_common.m_logFileOverwrite)->default_value(true, "yes"),
                    "")("worker.log_level", boost::program_options::value<unsigned short>(&m_options.m_worker.m_common.m_logLevel)->default_value(1), "")(
                "worker.set_my_rootsys", boost::program_options::value<bool>(&m_options.m_worker.m_setMyROOTSYS)->default_value(true, "yes"), "")(
                "worker.my_rootsys", boost::program_options::value<std::string>(&m_options.m_worker.m_myROOTSYS)->default_value("$ROOTSYS"), "")(
                "worker.agent_shutdown_if_idle_for_sec",
                boost::program_options::value<int>(&m_options.m_worker.m_common.m_shutdownIfIdleForSec)->default_value(1800),
                "")("worker.xproof_ports_range_min",
                    boost::program_options::value<unsigned int>(&m_options.m_worker.m_common.m_xproofPortsRangeMin)->default_value(21001))(
                "worker.xproof_ports_range_max",
                boost::program_options::value<unsigned int>(&m_options.m_worker.m_common.m_xproofPortsRangeMax)->default_value(22000))(
                "worker.agent_node_readbuffer",
                boost::program_options::value<unsigned int>(&m_options.m_worker.m_common.m_agentNodeReadBuffer)->default_value(5000));
            config_file_options.add_options()(
                "lsf_plugin.email_job_output", boost::program_options::value<bool>(&m_options.m_lsf.m_emailOutput)->default_value(false, "no"), "")(
                "lsf_plugin.upload_job_log", boost::program_options::value<bool>(&m_options.m_lsf.m_uploadJobLog)->default_value(false, "no"), "");
            config_file_options.add_options()(
                "pbs_plugin.upload_job_log", boost::program_options::value<bool>(&m_options.m_pbs.m_uploadJobLog)->default_value(false, "no"), "")(
                "pbs_plugin.options_file",
                boost::program_options::value<std::string>(&m_options.m_pbs.m_optionsFile)->default_value("$POD_LOCATION/etc/Job.pbs.option"),
                "");
            config_file_options.add_options()(
                "ge_plugin.upload_job_log", boost::program_options::value<bool>(&m_options.m_oge.m_uploadJobLog)->default_value(false, "no"), "")(
                "ge_plugin.options_file",
                boost::program_options::value<std::string>(&m_options.m_oge.m_optionsFile)->default_value("$POD_LOCATION/etc/Job.oge.option"),
                "");
            config_file_options.add_options()(
                "condor_plugin.upload_job_log", boost::program_options::value<bool>(&m_options.m_condor.m_uploadJobLog)->default_value(false, "no"), "")(
                "condor_plugin.options_file",
                boost::program_options::value<std::string>(&m_options.m_condor.m_optionsFile)->default_value("$POD_LOCATION/etc/Job.condor.option"),
                "");

            config_file_options.add_options()(
                "slurm_plugin.upload_job_log", boost::program_options::value<bool>(&m_options.m_slurm.m_uploadJobLog)->default_value(false, "no"), "");

            if (!_get_default)
            {
                std::ifstream ifs(_PoDCfgFileName.c_str());
                if (!ifs.good())
                {
                    std::string msg("Could not open a PoD configuration file: ");
                    msg += _PoDCfgFileName;
                    throw std::runtime_error(msg);
                }
                // Parse the config file
                // TODO: use allow_unregistered when switched to boost 1.35
                boost::program_options::store(boost::program_options::parse_config_file(ifs, config_file_options), m_keys);
            }
            else
            {
                // we fake reading of arguments, just to get a default values of all keys
                char* arg[1];
                arg[0] = new char[1];
                arg[0][0] = '\0';
                boost::program_options::store(boost::program_options::basic_command_line_parser<char>(1, arg).options(config_file_options).run(), m_keys);
                delete[] arg[0];
            }

            boost::program_options::notify(m_keys);
        }

        std::string getValueForKey(const std::string& _Key) const
        {
            return convertAnyToString(m_keys[_Key].value());
        }

        /// Returns strings "yes" or "no". Returns an empty string (if key is not of type bool)
        std::string getUnifiedBoolValueForBoolKey(const std::string& _Key) const
        {
            if (m_keys[_Key].value().type() != typeid(bool))
                return ("");

            return (m_keys[_Key].as<bool>() ? "yes" : "no");
        }

        const SPoDUserDefaultsOptions_t getOptions() const
        {
            return m_options;
        }

        static void printDefaults(std::ostream& _stream)
        {
            CPoDUserDefaults ud;
            ud.init("", true);

            _stream << "[server]\n"
                    << "work_dir=" << ud.getValueForKey("server.work_dir") << "\n"
                    << "sandbox_dir=" << ud.getValueForKey("server.sandbox_dir") << "\n"
                    << "logfile_dir=" << ud.getValueForKey("server.logfile_dir") << "\n"
                    << "logfile_overwrite=" << ud.getUnifiedBoolValueForBoolKey("server.logfile_overwrite") << "\n"
                    << "log_level=" << ud.getValueForKey("server.log_level") << "\n"
                    << "agent_shutdown_if_idle_for_sec=" << ud.getValueForKey("server.agent_shutdown_if_idle_for_sec") << "\n"
                    << "agent_local_client_port_min=" << ud.getValueForKey("server.agent_local_client_port_min") << "\n"
                    << "agent_local_client_port_max=" << ud.getValueForKey("server.agent_local_client_port_max") << "\n"
                    << "xproof_ports_range_min=" << ud.getValueForKey("server.xproof_ports_range_min") << "\n"
                    << "xproof_ports_range_max=" << ud.getValueForKey("server.xproof_ports_range_max") << "\n"
                    << "agent_ports_range_min=" << ud.getValueForKey("server.agent_ports_range_min") << "\n"
                    << "agent_ports_range_max=" << ud.getValueForKey("server.agent_ports_range_max") << "\n"
                    << "agent_threads=" << ud.getValueForKey("server.agent_threads") << "\n"
                    << "agent_node_readbuffer=" << ud.getValueForKey("server.agent_node_readbuffer") << "\n"
                    << "packet_forwarding=" << ud.getValueForKey("server.packet_forwarding") << "\n"
                    << "proof_cfg_entry_pattern=" << ud.getValueForKey("server.proof_cfg_entry_pattern") << "\n"
                    << "gen_temp_ssh_keys=" << ud.getUnifiedBoolValueForBoolKey("server.gen_temp_ssh_keys") << "\n";
            _stream << "[worker]\n"
                    << "work_dir=" << ud.getValueForKey("worker.work_dir") << "\n"
                    << "logfile_dir=" << ud.getValueForKey("worker.logfile_dir") << "\n"
                    << "logfile_overwrite=" << ud.getUnifiedBoolValueForBoolKey("worker.logfile_overwrite") << "\n"
                    << "log_level=" << ud.getValueForKey("worker.log_level") << "\n"
                    << "set_my_rootsys=" << ud.getUnifiedBoolValueForBoolKey("worker.set_my_rootsys") << "\n"
                    << "my_rootsys=" << ud.getValueForKey("worker.my_rootsys") << "\n"
                    << "agent_shutdown_if_idle_for_sec=" << ud.getValueForKey("worker.agent_shutdown_if_idle_for_sec") << "\n"
                    << "xproof_ports_range_min=" << ud.getValueForKey("worker.xproof_ports_range_min") << "\n"
                    << "xproof_ports_range_max=" << ud.getValueForKey("worker.xproof_ports_range_max") << "\n"
                    << "agent_node_readbuffer=" << ud.getValueForKey("worker.agent_node_readbuffer") << "\n";
            _stream << "[lsf_plugin]\n"
                    << "email_job_output=" << ud.getUnifiedBoolValueForBoolKey("lsf_plugin.email_job_output") << "\n"
                    << "upload_job_log=" << ud.getUnifiedBoolValueForBoolKey("lsf_plugin.upload_job_log") << "\n";
            _stream << "[pbs_plugin]\n"
                    << "upload_job_log=" << ud.getUnifiedBoolValueForBoolKey("pbs_plugin.upload_job_log") << "\n"
                    << "options_file=" << ud.getValueForKey("pbs_plugin.options_file") << "\n";
            _stream << "[ge_plugin]\n"
                    << "upload_job_log=" << ud.getUnifiedBoolValueForBoolKey("ge_plugin.upload_job_log") << "\n"
                    << "options_file=" << ud.getValueForKey("ge_plugin.options_file") << "\n";
            _stream << "[condor_plugin]\n"
                    << "upload_job_log=" << ud.getUnifiedBoolValueForBoolKey("condor_plugin.upload_job_log") << "\n"
                    << "options_file=" << ud.getValueForKey("condor_plugin.options_file") << "\n";
            _stream << "[slurm_plugin]\n"
                    << "upload_job_log=" << ud.getUnifiedBoolValueForBoolKey("slurm_plugin.upload_job_log") << "\n";
        }

    private:
        boost::program_options::variables_map m_keys;
        SPoDUserDefaultsOptions_t m_options;
    };

//=============================================================================
#if defined(BOOST_PROPERTY_TREE)
    struct SPoDRemoteOptions
    {
        std::string m_connectionString;
        std::string m_PoDLocation;
        std::string m_envLocal;
        std::string m_envRemote;
        std::string m_envSSHOpenDomain;
        size_t m_localAgentPort;
        size_t m_localXpdPort;
        size_t m_localMainTunnelPort;

        SPoDRemoteOptions()
            : m_localAgentPort(0)
            , m_localXpdPort(0)
            , m_localMainTunnelPort(0)
        {
        }

        void load(const std::string& _filename)
        {
            // Create an empty property tree object
            using boost::property_tree::ptree;
            ptree pt;

            try
            {
                read_ini(_filename, pt);
                m_connectionString = pt.get<std::string>("pod-remote.connectionString");
                m_PoDLocation = pt.get<std::string>("pod-remote.PoDLocation");
                m_envLocal = pt.get<std::string>("pod-remote.envLocal");
                m_envRemote = pt.get<std::string>("pod-remote.envRemote");
                m_envSSHOpenDomain = pt.get<std::string>("pod-remote.envSSHOpenDomain");
                m_localAgentPort = pt.get<size_t>("pod-remote.localAgentPort");
                m_localXpdPort = pt.get<size_t>("pod-remote.localXpdPort");
                m_localMainTunnelPort = pt.get<size_t>("pod-remote.localMainTunnelPort");
            }
            catch (...)
            {
                // ignore missing nodes
            }
        }
        void save(const std::string& _filename)
        {
            // Create an empty property tree object
            using boost::property_tree::ptree;
            ptree pt;

            pt.put("pod-remote.connectionString", m_connectionString);
            pt.put("pod-remote.PoDLocation", m_PoDLocation);
            pt.put("pod-remote.envLocal", m_envLocal);
            pt.put("pod-remote.envRemote", m_envRemote);
            pt.put("pod-remote.envSSHOpenDomain", m_envSSHOpenDomain);
            pt.put("pod-remote.localAgentPort", m_localAgentPort);
            pt.put("pod-remote.localXpdPort", m_localXpdPort);
            pt.put("pod-remote.localMainTunnelPort", m_localMainTunnelPort);

            // Write the property tree to the XML file.
            write_ini(_filename, pt);
        }
    };
    struct SPoDSSHOptions
    {
        std::string m_config;

        void load(const std::string& _filename)
        {
            // Create an empty property tree object
            using boost::property_tree::ptree;
            ptree pt;
            try
            {
                read_ini(_filename, pt);
                m_config = pt.get<std::string>("pod-ssh.config");
            }
            catch (...)
            {
                // ignore missing nodes
            }
        }
        void save(const std::string& _filename)
        {
            // Create an empty property tree object
            using boost::property_tree::ptree;
            ptree pt;

            pt.put("pod-ssh.config", m_config);

            // Write the property tree to the XML file.
            write_ini(_filename, pt);
        }
    };

#endif

    inline std::string showWrkPackageDir(CPoDUserDefaults* _ud = NULL)
    {
        std::string sSandboxDir;
        if (NULL == _ud)
        {
            CPoDUserDefaults ud;
            std::string podCFG(showCurrentPUDFile());
            MiscCommon::smart_path(&podCFG);
            ud.init(podCFG);
            sSandboxDir = ud.getValueForKey("server.sandbox_dir");
            if (sSandboxDir.empty())
                sSandboxDir = ud.getValueForKey("server.work_dir");
        }
        else
        {
            sSandboxDir = _ud->getValueForKey("server.sandbox_dir");
            if (sSandboxDir.empty())
                sSandboxDir = _ud->getValueForKey("server.work_dir");
        }

        MiscCommon::smart_path(&sSandboxDir);
        MiscCommon::smart_append(&sSandboxDir, '/');
        return (sSandboxDir + "wrk/");
    }
    inline std::string showWrkPackage(CPoDUserDefaults* _ud = NULL)
    {
        return (showWrkPackageDir(_ud) + "pod-worker");
    }
    inline std::string showWrkScript(CPoDUserDefaults* _ud = NULL)
    {
        return (showWrkPackageDir(_ud) + "PoDWorker.sh");
    }
}

#endif /* PODUSERDEFAULTSOPTIONS_H_ */
