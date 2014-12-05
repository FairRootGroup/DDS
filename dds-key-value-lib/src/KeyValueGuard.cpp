// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "KeyValueGuard.h"
#include "UserDefaults.h"
#include "BOOST_FILESYSTEM.h"
// BOOST
#include <boost/property_tree/ini_parser.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
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

void CKeyValueGuard::clean() const
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

void CKeyValueGuard::init()
{
    Logger::instance().init(); // Initialize log

    // create cfg file if missing
    fs::path cfgFile(getCfgFilePath());
    if (!fs::exists(cfgFile))
        ofstream f(cfgFile.generic_string());

    boost::property_tree::ini_parser::read_ini(cfgFile.generic_string(), m_pt);
}

const std::string CKeyValueGuard::getCfgFilePath() const
{
    boost::filesystem::path cfgFile(CUserDefaults::instance().getDDSPath());
    cfgFile /= "task.cfg";
    return cfgFile.generic_string();
}

void CKeyValueGuard::putValue(const std::string& _key, const std::string& _value, const std::string& _taskId)
{
    boost::interprocess::file_lock f_lock(getCfgFilePath().c_str());
    const string sKey = _key + "." + _taskId;
    {
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(f_lock);
        m_pt.put(sKey, _value);
        boost::property_tree::ini_parser::write_ini(getCfgFilePath(), m_pt);
    }
}

void CKeyValueGuard::putValue(const std::string& _key, const std::string& _value)
{
    boost::interprocess::file_lock f_lock(getCfgFilePath().c_str());
    const string sKey = _key;
    {
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(f_lock);
        m_pt.put(sKey, _value);
        boost::property_tree::ini_parser::write_ini(getCfgFilePath(), m_pt);
    }
}

void CKeyValueGuard::getValue(const std::string& _key, std::string* _value, const std::string& _taskId)
{
    if (_value == nullptr)
        throw invalid_argument("CKeyValueGuard::getValue: Value can't be NULL");

    boost::interprocess::file_lock f_lock(getCfgFilePath().c_str());
    const string sKey = _key + "." + _taskId;
    {
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(f_lock);
        fs::path cfgFile(getCfgFilePath());
        boost::property_tree::ini_parser::read_ini(cfgFile.generic_string(), m_pt);
        m_pt.get(sKey, *_value);
    }
}

void CKeyValueGuard::getValues(const std::string& _key, valuesMap_t* _values)
{
    if (_values == nullptr)
        throw invalid_argument("CKeyValueGuard::getValue: _values can't be NULL");

    boost::interprocess::file_lock f_lock(getCfgFilePath().c_str());
    boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(f_lock);
    fs::path cfgFile(getCfgFilePath());
    boost::property_tree::ini_parser::read_ini(cfgFile.generic_string(), m_pt);
    try
    {

        for (auto& element : m_pt.get_child(_key))
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
    std::lock_guard<std::mutex> lock(m_mtxAgentConnnection);
    if (!m_agentConnectionMng || m_agentConnectionMng->stopped())
    {
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
