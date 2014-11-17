// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "KeyValueGuard.h"
#include "UserDefaults.h"
#include "BOOST_FILESYSTEM.h"
// BOOST
#include <boost/property_tree/ini_parser.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace dds;
using namespace std;

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

void CKeyValueGuard::putValue(const std::string& _key, const std::string& _value)
{
    boost::interprocess::file_lock f_lock(getCfgFilePath().c_str());
    m_pt.put(_key, _value);
    {
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> e_lock(f_lock);
        boost::property_tree::ini_parser::write_ini(getCfgFilePath(), m_pt);
    }
}
