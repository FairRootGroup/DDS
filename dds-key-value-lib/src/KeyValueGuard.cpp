// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "KeyValueGuard.h"
#include "UserDefaults.h"
#include "BOOST_FILESYSTEM.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace std;
using namespace dds;
using namespace MiscCommon;
namespace ip = boost::interprocess;

CKeyValueGuard::CKeyValueGuard()
{
    Logger::instance().init(); // Initialize log

    boost::filesystem::path cfgFile(CUserDefaults::instance().getDDSPath());
    cfgFile /= "task.cfg";
    m_sCfgFilePath = cfgFile.generic_string();
}

CKeyValueGuard::~CKeyValueGuard()
{
}

CKeyValueGuard& CKeyValueGuard::instance()
{
    static CKeyValueGuard instance;
    return instance;
}

void CKeyValueGuard::createStorage()
{
    // create cfg file if missing
    fs::path cfgFile(getCfgFilePath());
    if (fs::exists(cfgFile))
    {
        LOG(debug) << "Removing key-value storage file: " << cfgFile.generic_string();
        if (!fs::remove(cfgFile))
            throw runtime_error("Failed to remove key-value storage file: " + cfgFile.generic_string());
    }

    LOG(debug) << "Create key-value storage file: " << cfgFile.generic_string();
    ofstream f(cfgFile.generic_string());

    // Named mutex
    char* ddsTaskId;
    ddsTaskId = getenv("DDS_TASK_ID");
    if (NULL == ddsTaskId)
        throw runtime_error("Can't initialize semaphore because DDS_TASK_ID variable is not set");
    const string mutexName(ddsTaskId);

    const bool removed = ip::named_mutex::remove(mutexName.c_str());
    LOG(debug) << "Shared object remove status: " << removed;

    m_fileMutex.reset();
    try
    {
        LOG(debug) << "Creating key-value lock object with name " << mutexName;
        m_fileMutex = make_shared<ip::named_mutex>(ip::open_or_create, mutexName.c_str());
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(fatal) << "Can't initialize key-value lock object with name " << mutexName << ": " << _e.what();
    }
}

void CKeyValueGuard::initLock()
{
    if (m_fileMutex)
        return;

    // Named mutex
    char* ddsTaskId;
    ddsTaskId = getenv("DDS_TASK_ID");
    if (NULL == ddsTaskId)
        throw runtime_error("Can't initialize semaphore because DDS_TASK_ID variable is not set");
    const string mutexName(ddsTaskId);

    try
    {
        LOG(debug) << "Creating key-value lock object with name " << mutexName;
        m_fileMutex = make_shared<ip::named_mutex>(ip::open_only, mutexName.c_str());
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(fatal) << "Can't initialize key-value lock object with name " << mutexName << ": " << _e.what();
    }
}

const std::string CKeyValueGuard::getCfgFilePath() const
{
    return m_sCfgFilePath;
}

void CKeyValueGuard::putValue(const std::string& _key, const std::string& _value, const std::string& _taskId)
{
    const string sKey = _key + "." + _taskId;
    putValue(sKey, _value);
}

void CKeyValueGuard::putValue(const std::string& _key, const std::string& _value)
{
    LOG(debug) << "CKeyValueGuard::putValue key=" << _key << " value=" << _value << " ...";
    const string sKey = _key;
    {
        boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*m_fileMutex);
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(getCfgFilePath(), pt);
        pt.put(sKey, _value);
        boost::property_tree::ini_parser::write_ini(getCfgFilePath(), pt);
    }
    LOG(debug) << "CKeyValueGuard::putValue key=" << _key << " value=" << _value << " done";
}

void CKeyValueGuard::getValue(const std::string& _key, std::string* _value, const std::string& _taskId)
{
    LOG(debug) << "CKeyValueGuard::getValue key=" << _key << " taskId=" << _taskId << " ...";
    if (_value == nullptr)
        throw invalid_argument("CKeyValueGuard::getValue: Value can't be NULL");

    const string sKey = _key + "." + _taskId;
    // IMPORTANT: adding additional scope to make sure that the stream has flushed the data
    {
        boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*m_fileMutex);
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(getCfgFilePath(), pt);
        pt.get(sKey, *_value);
    }
    LOG(debug) << "CKeyValueGuard::getValue key=" << _key << " taskId=" << _taskId << " done";
}

void CKeyValueGuard::getValues(const std::string& _key, valuesMap_t* _values)
{
    LOG(debug) << "CKeyValueGuard::getValues key=" << _key << " ...";
    if (_values == nullptr)
        throw invalid_argument("CKeyValueGuard::getValue: _values can't be NULL");

    _values->clear();

    boost::property_tree::ptree pt;

    {
        boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*m_fileMutex);
        boost::property_tree::ini_parser::read_ini(getCfgFilePath(), pt);
    }

    try
    {
        for (auto& element : pt.get_child(_key))
        {
            _values->insert(make_pair(element.first, element.second.get_value<std::string>()));
        }
    }
    catch (...)
    {
        LOG(error) << "CKeyValueGuard::getValues key=" << _key << " exception";
        return;
    }
    LOG(debug) << "CKeyValueGuard::getValues key=" << _key << " done";
}

void CKeyValueGuard::initAgentConnection()
{
    LOG(debug) << "CKeyValueGuard::initAgentConnection: is going to init";
    if (!m_agentConnectionMng || m_agentConnectionMng->stopped())
    {
        LOG(debug) << "CKeyValueGuard::initAgentConnection: start init";

        m_agentConnectionMng.reset();
        m_agentConnectionMng = make_shared<CAgentConnectionManager>();
        m_agentConnectionMng->m_syncHelper = &m_syncHelper;

        // Don't block main thread, start transport service in a thread
        std::thread t([this]()
                      {
                          try
                          {
                              m_agentConnectionMng->start();
                          }
                          catch (exception& _e)
                          {
                              LOG(fatal) << "AgentConnectionManager: exception in the transport service: " << _e.what();
                          }
                      });
        t.detach();
    }
}

int CKeyValueGuard::updateKey(const SUpdateKeyCmd& _cmd)
{
    if (!m_agentConnectionMng || m_agentConnectionMng->stopped())
    {
        LOG(error) << "CKeyValueGuard::updateKey: Agent connection channel is not running. Failed to update " << _cmd;
        return 1;
    }

    LOG(debug) << "CKeyValueGuard::updateKey: sending key update: " << _cmd;
    return m_agentConnectionMng->updateKey(_cmd);
}
