// Copyright 2014 GSI, Inc. All rights reserved.
//
// TODO: Describe
//
#include "UserDefaults.h"
// BOOST
#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
// MiscCommon
#include "FindCfgFile.h"
#include "Process.h"
#include "SessionIDFile.h"
#include "Version.h"
// STD
#include <iomanip>

using namespace std;
using namespace dds;
using namespace dds::user_defaults_api;
using namespace dds::misc;
namespace fs = boost::filesystem;

CUserDefaults::CUserDefaults(const boost::uuids::uuid& _sid)
{
    makeDefaultDirs();
    setSessionID(_sid);
    init();
}

CUserDefaults::~CUserDefaults()
{
}

CUserDefaults& CUserDefaults::instance(const boost::uuids::uuid& _sid)
{
    static CUserDefaults instance(_sid);
    return instance;
}

boost::uuids::uuid CUserDefaults::getInitialSID()
{
    return boost::uuids::string_generator()("11111111-1111-1111-1111-111111111111");
}

void CUserDefaults::reinit(const boost::uuids::uuid& _sid, const string& _cfgFileName, bool _get_default)
{
    setSessionID(_sid);
    init(_cfgFileName, _get_default);
}

void CUserDefaults::init(const string& _cfgFileName, bool _get_default)
{
    m_keys.clear();
    boost::program_options::options_description config_file_options("DDS user defaults options");
    config_file_options.add_options()(
        "server.work_dir",
        boost::program_options::value<string>(&m_options.m_server.m_workDir)->default_value("$HOME/.DDS"),
        "");
    config_file_options.add_options()(
        "server.sandbox_dir",
        boost::program_options::value<string>(&m_options.m_server.m_sandboxDir)->default_value("$HOME/.DDS"),
        "");
    config_file_options.add_options()(
        "server.log_dir",
        boost::program_options::value<string>(&m_options.m_server.m_logDir)->default_value("$HOME/.DDS/log"),
        "");
    config_file_options.add_options()(
        "server.log_severity_level",
        boost::program_options::value<ELogSeverityLevel>(&m_options.m_server.m_logSeverityLevel)->default_value(info));
    config_file_options.add_options()(
        "server.log_rotation_size",
        boost::program_options::value<unsigned int>(&m_options.m_server.m_logRotationSize)->default_value(10));
    config_file_options.add_options()(
        "server.log_has_console_output",
        boost::program_options::value<bool>(&m_options.m_server.m_logHasConsoleOutput)->default_value(true));
    config_file_options.add_options()(
        "server.commander_port_range_min",
        boost::program_options::value<unsigned int>(&m_options.m_server.m_ddsCommanderPortRangeMin)
            ->default_value(20000),
        "");
    config_file_options.add_options()(
        "server.commander_port_range_max",
        boost::program_options::value<unsigned int>(&m_options.m_server.m_ddsCommanderPortRangeMax)
            ->default_value(21000),
        "");
    config_file_options.add_options()(
        "server.idle_time",
        boost::program_options::value<unsigned int>(&m_options.m_server.m_idleTime)->default_value(1800));
    config_file_options.add_options()(
        "agent.work_dir", boost::program_options::value<string>(&m_options.m_agent.m_workDir)->default_value(""), "");
    // default is "-rw-rw----", i.e. 0660
    config_file_options.add_options()(
        "agent.access_permissions",
        boost::program_options::value<string>(&m_options.m_agent.m_accessPermissions)->default_value("0660"),
        "");
    config_file_options.add_options()(
        "agent.disk_space_threshold",
        boost::program_options::value<unsigned int>(&m_options.m_agent.m_diskSpaceThreshold)->default_value(500),
        "");

    if (!_get_default)
    {
        ifstream ifs(_cfgFileName.c_str());
        if (!ifs.good())
        {
            string msg("Could not open a DDS configuration file: ");
            msg += _cfgFileName;
            throw runtime_error(msg);
        }
        // Parse the config file
        boost::program_options::store(boost::program_options::parse_config_file(ifs, config_file_options, true),
                                      m_keys);
    }
    else
    {
        // we fake reading of arguments, just to get a default values of all keys
        char* arg[1];
        arg[0] = new char[1];
        arg[0][0] = '\0';
        boost::program_options::store(
            boost::program_options::basic_command_line_parser<char>(1, arg).options(config_file_options).run(), m_keys);
        delete[] arg[0];
    }

    boost::program_options::notify(m_keys);

    if (!_get_default)
    {
        // Update paths with sessions ID
        m_options.m_server.m_workDir_NoSID = m_options.m_server.m_workDir;
        addSessionIDtoPath(m_options.m_server.m_workDir);
        addSessionIDtoPath(m_options.m_server.m_sandboxDir);
        addSessionIDtoPath(m_options.m_server.m_logDir);
    }
}

void CUserDefaults::init(bool _get_default)
{
    // Use the default look up algorithm
    string sDDSCfgFileName(currentUDFile());
    init(sDDSCfgFileName, _get_default);
}

void CUserDefaults::makeDefaultDirs()
{
    if (fs::exists(CUserDefaults::currentUDFile()))
        return;

    string localDir{ "$HOME/.DDS" };
    smart_path(&localDir);
    if (!fs::exists(fs::path(localDir)) && !fs::create_directories(fs::path(localDir)))
    {
        stringstream ss;
        ss << "Failed to create the default directory " << quoted(localDir) << " for user defaults";
        throw runtime_error(ss.str());
    }

    string cfgFilepath{ localDir + "/DDS.cfg" };
    smart_path(&cfgFilepath);
    cout << "Generating a default DDS configuration file..." << endl;
    ofstream f(cfgFilepath.c_str());
    if (!f.is_open())
    {
        stringstream ss;
        ss << "Failed to open a default configuration file " << quoted(cfgFilepath) << " for writing";
        throw runtime_error(ss.str());
    }

    f << "# DDS user defaults\n"
      << "# version: " << DDS_USER_DEFAULTS_CFG_VERSION_STRING << "\n"
      << "#\n"
      << "# Please use DDS User's Manual to find out more details on\n"
      << "# keys and values of this configuration file.\n"
      << "# DDS User's Manual can be found in $DDS_LOCATION/doc folder or\n"
      << "# by the following address: http://dds.gsi.de/documentation.html\n";
    CUserDefaults::printDefaults(f);
    cout << "Generating a default DDS configuration file - DONE." << endl;
}

void CUserDefaults::setSessionID(const boost::uuids::uuid& _sid)
{
    if (_sid == getInitialSID())
    {
        m_sessionID.clear();
        return;
    }

    if (!_sid.is_nil())
        m_sessionID = boost::lexical_cast<std::string>(_sid);
    else
    {
        string sid = getDefaultSID();
        if (!sid.empty())
            m_sessionID = sid;
    }
}

void CUserDefaults::printDefaults(ostream& _stream)
{
    CUserDefaults ud(boost::uuids::nil_uuid());
    ud.init(true);

    _stream << "[server]\n"
            << "work_dir=" << ud.getValueForKey("server.work_dir") << "\n"
            << "sandbox_dir=" << ud.getValueForKey("server.sandbox_dir") << "\n"
            << "log_dir=" << ud.getValueForKey("server.log_dir") << "\n"
            << "#\n"
            << "# Log severity can be one of the following values:\n"
            << "# p_l, p_m, p_h, dbg, inf, wrn, err, fat\n"
            << "# p_l - protocol low level events and higher\n"
            << "# p_m - protocol middle level events and higher\n"
            << "# p_h - protocol high level events and higher\n"
            << "# dbg - general debug events and higher\n"
            << "# inf - info events and higher\n"
            << "# wrn - warning events and higher\n"
            << "# err - error events and higher\n"
            << "# fat - fatal errors and higher\n"
            << "#\n"
            << "log_severity_level=" << ud.getValueForKey("server.log_severity_level") << "\n"
            << "log_rotation_size=" << ud.getValueForKey("server.log_rotation_size") << "\n"
            << "log_has_console_output=" << ud.getValueForKey("server.log_has_console_output") << "\n"
            << "commander_port_range_min=" << ud.getValueForKey("server.commander_port_range_min") << "\n"
            << "commander_port_range_max=" << ud.getValueForKey("server.commander_port_range_max") << "\n"
            << "idle_time=" << ud.getValueForKey("server.idle_time") << "\n"
            << "\n\n[agent]\n"
            << "# This option can help to relocate the work directory of agents.\n"
            << "# The option is ignored by the localhost and ssh plug-ins.\n"
            << "# By default the wrk dir is placed inside the path specified by server.sandbox_dir.\n"
            << "# It's recommended to keep this option empty.\n"
            << "work_dir=" << ud.getValueForKey("agent.work_dir") << "\n"
            << "#\n"
            << "# This option forces the given file mode on agent side files.\n"
            << "# At the moment the access permissions are applied only on user task log files (stdout and stderr).\n"
            << "# Mode can be specified with octal numbers.\n"
            << "# 0400 - Read by owner\n"
            << "# 0040 - Read by group\n"
            << "# 0004 - Read by world\n"
            << "# 0200 - Write by owner\n"
            << "# 0020 - Write by group\n"
            << "# 0002 - Write by world\n"
            << "# 0100 - execute by owner\n"
            << "# 0010 - execute by group\n"
            << "# 0001 - execute by world\n"
            << "# To combine these, just add the numbers together:\n"
            << "# 0444 - Allow read permission to owner and group and world\n"
            << "# 0777 - Allow everyone to read, write, and execute file\n"
            << "#\n"
            << "access_permissions=" << ud.getValueForKey("agent.access_permissions") << "\n"
            << "# The agent will trigger a self-shutdown if the free disk space is below this threshold.\n"
            << "# The value in MB. Default is 500 MB.\n"
            << "# Set it to 0 to disiable.\n"
            << "#\n"
            << "disk_space_threshold=" << ud.getValueForKey("agent.disk_space_threshold") << "\n";
}

string CUserDefaults::convertAnyToString(const boost::any& _any) const
{
    if (_any.type() == typeid(string))
        return boost::any_cast<string>(_any);

    ostringstream ss;
    if (_any.type() == typeid(int))
        ss << boost::any_cast<int>(_any);
    else if (_any.type() == typeid(unsigned int))
        ss << boost::any_cast<unsigned int>(_any);
    else if (_any.type() == typeid(unsigned char))
        ss << boost::any_cast<unsigned char>(_any);
    else if (_any.type() == typeid(unsigned short))
        ss << boost::any_cast<unsigned short>(_any);
    else if (_any.type() == typeid(bool))
        ss << boost::any_cast<bool>(_any);
    else if (_any.type() == typeid(ELogSeverityLevel))
        ss << reinterpret_cast<ELogSeverityLevel>(boost::any_cast<ELogSeverityLevel>(_any));

    return ss.str();
}

string CUserDefaults::getValueForKey(const string& _key) const
{
    string ret = convertAnyToString(m_keys[_key].value());
    // Working dir of agents is always their home (DDS_LOCATION)
    if (_key == "server.work_dir" && isAgentInstance())
    {
        return getDDSPath();
    }

    if ((_key == "server.work_dir" || _key == "server.sandbox_dir" || _key == "server.log_dir") && !m_sessionID.empty())
    {
        addSessionIDtoPath(ret);
    }

    return ret;
}

/// Returns DDS working directory. For agents it is always $DDS_LOCATION
std::string CUserDefaults::getWrkDir() const
{
    string sWrkDir(getValueForKey("server.work_dir"));
    smart_path(&sWrkDir);
    smart_append(&sWrkDir, '/');

    return sWrkDir;
}

/// Returns strings "yes" or "no". Returns an empty string (if key is not of type bool)
string CUserDefaults::getUnifiedBoolValueForBoolKey(const string& _Key) const
{
    if (m_keys[_Key].value().type() != typeid(bool))
        return ("");

    return (m_keys[_Key].as<bool>() ? "yes" : "no");
}

const SDDSUserDefaultsOptions_t CUserDefaults::getOptions() const
{
    return m_options;
}

string CUserDefaults::currentUDFile()
{
    CFindCfgFile<string> cfg;
    cfg.SetOrder("$HOME/.DDS/DDS.cfg")("$DDS_LOCATION/etc/DDS.cfg")("$DDS_LOCATION/DDS.cfg");
    string val;
    cfg.GetCfg(&val);

    smart_path(&val);

    return val;
}

string CUserDefaults::getDDSPath()
{
    char* dds_location;
    dds_location = getenv("DDS_LOCATION");
    if (NULL == dds_location)
        return string();

    string sDDSPath(dds_location);
    smart_path(&sDDSPath);
    smart_append(&sDDSPath, '/');
    return sDDSPath;
}

string CUserDefaults::getServerInfoFileLocationSrv() const
{
    const string sFileName(getServerInfoFileName());
    string sWrkDir(getValueForKey("server.work_dir"));
    smart_path(&sWrkDir);
    smart_append(&sWrkDir, '/');
    return (sWrkDir + sFileName);
}

string CUserDefaults::getServerInfoFileName() const
{
    return ("server_info.cfg");
}

string CUserDefaults::getServerInfoFileLocation() const
{
    const string sFileName(getServerInfoFileName());
    CFindCfgFile<string> cfg;
    const string p1("$DDS_LOCATION/" + sFileName);
    const string p2(getServerInfoFileLocationSrv());

    cfg.SetOrder(p1)(p2);
    string val;
    cfg.GetCfg(&val);

    smart_path(&val);
    return val;
}

string CUserDefaults::getWrkPkgRootDir() const
{
    string sSandboxDir;
    sSandboxDir = getValueForKey("server.sandbox_dir");
    if (sSandboxDir.empty())
        sSandboxDir = getValueForKey("server.work_dir");

    smart_path(&sSandboxDir);
    smart_append(&sSandboxDir, '/');
    return (sSandboxDir + "wrk/");
}

string CUserDefaults::getWrkPkgDir(const string& _SubmissionID) const
{
    return (getWrkPkgRootDir() + _SubmissionID + "/");
}

string CUserDefaults::getWrkPkgPath(const string& _SubmissionID) const
{
    return (getWrkPkgDir(_SubmissionID) + "dds-worker");
}

string CUserDefaults::getWrkScriptPath(const string& _SubmissionID) const
{
    return (getWrkPkgDir(_SubmissionID) + "DDSWorker.sh");
}

string CUserDefaults::getUserEnvScript() const
{
    CFindCfgFile<string> cfg;
    cfg.SetOrder("$HOME/.DDS/user_worker_env.sh")("$DDS_LOCATION/etc/user_worker_env.sh");
    string val;
    cfg.GetCfg(&val);
    smart_path(&val);
    return val;
}

string CUserDefaults::getAgentIDFilePath()
{
    fs::path pathFile(getDDSPath());
    pathFile /= CUserDefaults::getAgentIDFileName();
    return (pathFile.string());
}

string CUserDefaults::getAgentIDFileName()
{
    return "dds-agent.client.id";
}

string CUserDefaults::getLogFile() const
{
    string sLogDir(getValueForKey("server.log_dir"));

    if (sLogDir.empty())
        throw runtime_error("Can't init Log engine. Log location is not specified. Make sure DDS environment is "
                            "properly initialised (echo $DDS_LOCATION).");

    smart_append<string>(&sLogDir, '/');
    string sLogFile(sLogDir);
    sLogFile += "dds_%Y-%m-%d.%N.log";
    smart_path<string>(&sLogFile);
    return sLogFile;
}

string CUserDefaults::getAgentLogStorageDir() const
{
    const string sAgentLogDir("log/agents/");
    string sWrkDir(getValueForKey("server.work_dir"));
    smart_path(&sWrkDir);
    smart_append(&sWrkDir, '/');
    return (sWrkDir + sAgentLogDir);
}

pid_t CUserDefaults::getScoutPid() const
{
    ifstream f(getDDSPath() + "DDSWorker.pid");
    pid_t nDDSScoutPid;
    f >> nDDSScoutPid;
    return nDDSScoutPid;
}

size_t CUserDefaults::getNumLeaderFW()
{
    return 4;
}

std::string CUserDefaults::getSMLeaderOutputName(uint64_t _protocolHeaderID) const
{
    // Shared memory for all messages addressed to commander
    string smName("DDSAO-");
    smName += to_string(_protocolHeaderID);
    return smName.substr(0, 24);
}

std::string CUserDefaults::getSMLeaderInputName(uint64_t _protocolHeaderID) const
{
    // Shared memory addressed to lobby leader
    // FIXME: maximum length of the SM name
    string smName("DDSAI-");
    smName += to_string(getScoutPid());
    smName += "-";
    smName += getLockedSID();
    std::string name = smName.substr(0, 24);
    return name + '_' + to_string(_protocolHeaderID % CUserDefaults::getNumLeaderFW());
}

std::vector<std::string> CUserDefaults::getSMLeaderInputNames() const
{
    // Shared memory for all messages addressed to commander
    string smName("DDSAI-");
    smName += to_string(getScoutPid());
    smName += "-";
    smName += getLockedSID();
    string baseName(smName.substr(0, 24));
    vector<string> names;
    for (size_t i = 0; i < CUserDefaults::getNumLeaderFW(); i++)
    {
        names.push_back(baseName + "_" + to_string(i));
    }
    return names;
}

string CUserDefaults::getPluginsRootDir() const
{
    stringstream ss;
    ss << getDDSPath() << "plugins/";
    return (ss.str());
}

string CUserDefaults::getPluginDir(const string& _path, const string& _pluginName) const
{
    stringstream ss;
    std::string path;
    if (!_path.empty())
    {
        path = _path;
        smart_path(&path);
        smart_append(&path, '/');
    }
    else
    {
        path = getPluginsRootDir();
    }
    ss << path << "dds-submit-" << _pluginName << "/";
    return (ss.str());
}

string CUserDefaults::getSIDName() const
{
    return "dds.sid";
}

/// Returns the full path to the main Session ID file
/// The function doesn't check wheather the file exists or not
string CUserDefaults::getMainSIDFile() const
{
    string sWorkDir(m_options.m_server.m_workDir);
    smart_path(&sWorkDir);

    // Main SID file is the file located on the commander's host
    fs::path pathMainSid(sWorkDir);
    pathMainSid /= getSIDName();
    return pathMainSid.string();
}

/// Returns Session ID full file path (return main SID if exists. If there is no main, it checks for a clone SID. If
/// none of SID exist, the fucntions returns an empty string)
string CUserDefaults::getSIDFile() const
{
    string sWorkDir(m_options.m_server.m_workDir);
    smart_path(&sWorkDir);

    // Main SID file is the file located on the commander's host
    fs::path pathMainSid(sWorkDir);
    pathMainSid /= getSIDName();
    if (fs::is_regular_file(pathMainSid))
        return pathMainSid.string();

    // SID clone file is the copy of the main SID for agents and other external process.
    fs::path pathCloneSid(getDDSPath());
    pathCloneSid /= getSIDName();
    if (fs::is_regular_file(pathCloneSid))
        return pathCloneSid.string();

    // could fined any SID file
    return string();
}

string CUserDefaults::getLockedSID() const
{
    // Get session ID from the local environment
    string sessionID("");
    std::string sidFile = getSIDFile();
    if (sidFile.empty())
    {
        throw runtime_error("Can't find SID file on the local system");
    }
    else
    {
        CSessionIDFile sid(sidFile);
        sessionID = sid.getLockedSID();
        if (sessionID.empty())
            throw runtime_error("Avaliable SID is empty");
    }
    return sessionID;
}

string CUserDefaults::getCurrentSID() const
{
    return m_sessionID;
}

void CUserDefaults::addSessionIDtoPath(std::string& _path) const
{
    if (m_sessionID.empty())
        return;

    fs::path pRet(_path);
    pRet /= getSessionsHolderDirName();
    pRet /= m_sessionID;
    _path = pRet.string();
}

void CUserDefaults::addSessionIDtoPath(std::string& _path, const boost::uuids::uuid& _sid) const
{
    if (_sid.is_nil())
        return;

    fs::path pRet(_path);
    pRet /= getSessionsHolderDirName();
    pRet /= boost::lexical_cast<string>(_sid);
    _path = pRet.string();
}

string CUserDefaults::getSessionsRootDir() const
{
    string val("$HOME/.DDS/");
    smart_path(&val);
    return val;
}

string CUserDefaults::getDefaultSIDLinkName() const
{
    string val(getSessionsRootDir());
    val += "default.sid";
    return val;
}

string CUserDefaults::getDefaultSID() const
{
    try
    {
        // Check whether users have defined DDS_SESSION_ID environment variable
        // If defined, use it instead of a default one
        char* pchDDSSessionID;
        pchDDSSessionID = getenv("DDS_SESSION_ID");
        if (pchDDSSessionID != NULL && strlen(pchDDSSessionID) > 0)
        {
            string sDDSSessionID(pchDDSSessionID);
            return sDDSSessionID;
        }

        // Get the default SID
        fs::path defaultSidLink(getDefaultSIDLinkName());
        if (!fs::exists(defaultSidLink) || !fs::is_symlink(defaultSidLink))
            return string();

        fs::path linkToPath(fs::read_symlink(defaultSidLink));
        if (linkToPath.empty())
            return string();

        boost::uuids::uuid sid = boost::uuids::string_generator()(linkToPath.filename().string());

        if (sid.is_nil())
            return string();

        return boost::lexical_cast<string>(sid);
    }
    catch (...)
    {
        return string();
    }
}

void CUserDefaults::setDefaultSID(const boost::uuids::uuid& _sid) const noexcept
{
    if (_sid.is_nil())
        return;

    string sessionID = boost::lexical_cast<string>(_sid);
    try
    {
        string sessionDir(getSessionsRootDir());
        addSessionIDtoPath(sessionDir);
        fs::path linkTo(sessionDir);
        fs::path linkFrom(getDefaultSIDLinkName());

        boost::system::error_code ec;
        // ignore errors during removal
        fs::remove(linkFrom, ec);
        // create a new symbolic link to the default SID
        fs::create_directory_symlink(linkTo, linkFrom);
    }
    catch (...)
    {
    }
}

string CUserDefaults::getSessionsHolderDirName() const
{
    return ("sessions");
}

string CUserDefaults::getCommanderPidFileName() const
{
    return "dds-commander.pid";
}

string CUserDefaults::getCommanderPidFile(const boost::uuids::uuid& _sid) const
{
    string sWorkDir;

    // TODO: Can be simplified once UD supports multiple sessions per instance
    if (_sid.is_nil())
        sWorkDir = CUserDefaults::instance().getOptions().m_server.m_workDir;
    else
    {
        sWorkDir = CUserDefaults::instance().getOptions().m_server.m_workDir_NoSID;
        addSessionIDtoPath(sWorkDir, _sid);
    }

    smart_path(&sWorkDir);
    fs::path pathPidFile(sWorkDir);
    pathPidFile /= CUserDefaults::instance().getCommanderPidFileName();

    return pathPidFile.string();
}

string CUserDefaults::getWnBinsDir() const
{
    stringstream ss;
    ss << getDDSPath() << "bin/wn_bins";
    return (ss.str());
}

string CUserDefaults::getTopologyXSDFilePath()
{
    return getDDSPath() + "share/topology.xsd";
}

bool CUserDefaults::isAgentInstance() const
{
    fs::path pathFile(getDDSPath());
    pathFile /= getServerInfoFileName();
    return fs::exists(pathFile);
}

bool CUserDefaults::IsSessionRunning(const boost::uuids::uuid& _sid) const
{
    bool bRunning(false);

    fs::path pathPidFile(CUserDefaults::instance().getCommanderPidFile(_sid));

    if (!fs::is_regular_file(pathPidFile))
        return bRunning;

    pid_t pid(0);
    ifstream f(pathPidFile.string());
    if (!f.is_open())
        return bRunning;
    f >> pid;

    bRunning = IsProcessRunning(pid);

    return bRunning;
}

std::string CUserDefaults::getSlotsRootDir() const
{
    boost::filesystem::path pathWrkDir(CUserDefaults::getDDSPath());
    pathWrkDir /= "slots";
    return pathWrkDir.string();
}
