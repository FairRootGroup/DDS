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

CKeyValueGuard::CKeyValueGuard()
{
    CUserDefaults::instance();
    init();
}

CKeyValueGuard::~CKeyValueGuard()
{
}

CKeyValueGuard& CKeyValueGuard::instance()
{
    static CKeyValueGuard instance;
    return instance;
}

void CKeyValueGuard::cleanStorage() const
{
    // create cfg file if missing
    fs::path cfgFile(getCfgFilePath());
    if (fs::exists(cfgFile))
    {
        LOG(debug) << "Removing key-value storage file: " << cfgFile.generic_string();
        if (!fs::remove(cfgFile))
            LOG(fatal) << "Failed to remove key-value storage file: " << cfgFile.generic_string();
    }
}

void CKeyValueGuard::createStorage()
{
    // create cfg file if missing
    boost::filesystem::path cfgFile(CUserDefaults::instance().getDDSPath());
    cfgFile /= "task.cfg";

    if (!fs::exists(cfgFile))
    {
        LOG(debug) << "Create key-value storage file: " << cfgFile.generic_string();
        ofstream f(cfgFile.generic_string());
    }
    m_sCfgFilePath = cfgFile.generic_string();

    // boost::property_tree::ini_parser::read_ini(cfgFile.generic_string(), m_pt);

    m_fileLock = boost::interprocess::file_lock(getCfgFilePath().c_str());
}

void CKeyValueGuard::init()
{
    Logger::instance().init(); // Initialize log
    createStorage();
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
    const string sKey = _key;
    boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(m_fileLock);
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(getCfgFilePath(), pt);
    pt.put(sKey, _value);
    boost::property_tree::ini_parser::write_ini(getCfgFilePath(), pt);
}

void CKeyValueGuard::getValue(const std::string& _key, std::string* _value, const std::string& _taskId)
{
    if (_value == nullptr)
        throw invalid_argument("CKeyValueGuard::getValue: Value can't be NULL");

    const string sKey = _key + "." + _taskId;
    {
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(m_fileLock);
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(getCfgFilePath(), pt);
        pt.get(sKey, *_value);
    }
}

void CKeyValueGuard::getValues(const std::string& _key, valuesMap_t* _values)
{
    if (_values == nullptr)
        throw invalid_argument("CKeyValueGuard::getValue: _values can't be NULL");

    _values->clear();

    boost::property_tree::ptree pt;
    boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(m_fileLock);
    boost::property_tree::ini_parser::read_ini(getCfgFilePath(), pt);

    try
    {
        for (auto& element : pt.get_child(_key))
        {
            _values->insert(make_pair(element.first, element.second.get_value<std::string>()));
        }
    }
    catch (...)
    {
        return;
    }
}

void CKeyValueGuard::initAgentConnection()
{
    LOG(debug) << "CKeyValueGuard::initAgentConnection: is going to init";
    std::lock_guard<std::mutex> lock(m_mtxAgentConnnection);
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
    std::lock_guard<std::mutex> lock(m_mtxAgentConnnection);
    return m_agentConnectionMng->updateKey(_cmd);
}
