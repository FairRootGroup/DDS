// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "KeyValueGuard.h"
#include "UserDefaults.h"
#include "BOOST_FILESYSTEM.h"
#include "AgentConnectionManager.h"
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
    const string sKey = _key + "_" + _taskId;
    {
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(f_lock);
        m_pt.put(_key, _value);
        boost::property_tree::ini_parser::write_ini(getCfgFilePath(), m_pt);
    }
}

void CKeyValueGuard::getValue(const std::string& _key, std::string* _value, const std::string& _taskId)
{
    if (_value == nullptr)
        throw invalid_argument("CKeyValueGuard::getValue: Value can't be NULL");

    boost::interprocess::file_lock f_lock(getCfgFilePath().c_str());
    const string sKey = _key + "_" + _taskId;
    {
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(f_lock);
        m_pt.get(_key, *_value);
    }
}

void CKeyValueGuard::notifyAgent(SCommandContainer* _newCommand)
{
    boost::asio::io_service io_service;
    CAgentConnectionManager agent(io_service);
    agent.m_cmdContainer = _newCommand;
    agent.start();
}
