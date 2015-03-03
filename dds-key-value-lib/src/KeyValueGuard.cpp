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

    // Create shared memory storage and semaphor to synchronize accesss to shared memory

    string storageName(to_string(CUserDefaults::instance().getScoutPid()));

    // Shared memory storage for property tree
    string sharedMemoryName(storageName);
    sharedMemoryName += "_DDSSM";

    const bool sharedMemoryRemoved = ip::shared_memory_object::remove(sharedMemoryName.c_str());
    LOG(debug) << "Shared memory " << sharedMemoryName << " remove status: " << sharedMemoryRemoved;

    m_sharedMemory.reset();
    try
    {
        LOG(debug) << "Creating key-value shared memory with name " << sharedMemoryName;
        m_sharedMemory =
            make_shared<ip::managed_shared_memory>(ip::open_or_create, sharedMemoryName.c_str(), 1024 * 1024);
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(fatal) << "Can't initialize key-value shared memory with name " << sharedMemoryName << ": " << _e.what();
    }

    // Named mutex
    string mutexName(storageName);
    mutexName += "_DDSM";

    const bool removed = ip::named_mutex::remove(mutexName.c_str());
    LOG(debug) << "Named mutex remove status: " << removed;

    m_sharedMemoryMutex.reset();
    try
    {
        LOG(debug) << "Creating key-value lock object with name " << mutexName;
        m_sharedMemoryMutex = make_shared<ip::named_mutex>(ip::open_or_create, mutexName.c_str());
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(fatal) << "Can't initialize key-value lock object with name " << mutexName << ": " << _e.what();
    }
}

void CKeyValueGuard::initLock()
{
    if (m_sharedMemoryMutex)
        return;

    string storageName(to_string(CUserDefaults::instance().getScoutPid()));

    // Shared memory storage for property tree
    string sharedMemoryName(storageName);
    sharedMemoryName += "_DDSSM";

    try
    {
        LOG(debug) << "Creating key-value shared memory with name " << sharedMemoryName;
        m_sharedMemory = make_shared<ip::managed_shared_memory>(ip::open_only, sharedMemoryName.c_str());
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(fatal) << "Can't initialize key-value shared memory with name " << sharedMemoryName << ": " << _e.what();
    }

    // Named mutex
    string mutexName(storageName);
    mutexName += "_DDSM";

    try
    {
        LOG(debug) << "Creating key-value lock object with name " << mutexName;
        m_sharedMemoryMutex = make_shared<ip::named_mutex>(ip::open_only, mutexName.c_str());
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
        try
        {
            ip::scoped_lock<ip::named_mutex> lock(*m_sharedMemoryMutex);

            sharedMemoryString_t* ptString = m_sharedMemory->find_or_construct<sharedMemoryString_t>("KeyValue")(
                sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

            sharedMemoryVectorStream_t readStream(sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

            readStream.swap_vector(*ptString);

            boost::property_tree::ptree ptsm;
            boost::property_tree::ini_parser::read_ini(readStream, ptsm);

            ptsm.put(sKey, _value);

            sharedMemoryVectorStream_t writeStream(sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

            boost::property_tree::ini_parser::write_ini(writeStream, ptsm);

            writeStream.swap_vector(*ptString);
            // LOG(debug) << "CKeyValueGuard::putValue file content |" << *ptString << "|";
            LOG(debug) << "Finish putValue key=" << _key << " value=" << _value;
        }
        catch (const ip::interprocess_exception& _e)
        {
            LOG(fatal) << "key-value guard putValue exception: " << _e.what();
        }
    }
    LOG(debug) << "CKeyValueGuard::putValue key=" << _key << " value=" << _value << " done";
}

void CKeyValueGuard::putValues(const std::vector<SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t>& _values)
{
    // LOG(debug) << "CKeyValueGuard::putValue key=" << _key << " value=" << _value << " ...";
    // const string sKey = _key;
    //{
    try
    {
        ip::scoped_lock<ip::named_mutex> lock(*m_sharedMemoryMutex);

        sharedMemoryString_t* ptString = m_sharedMemory->find_or_construct<sharedMemoryString_t>("KeyValue")(
            sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

        sharedMemoryVectorStream_t readStream(sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

        readStream.swap_vector(*ptString);

        boost::property_tree::ptree ptsm;
        boost::property_tree::ini_parser::read_ini(readStream, ptsm);

        for (const auto& v : _values)
        {
            ptsm.put(v->m_sKey, v->m_sValue);
        }

        sharedMemoryVectorStream_t writeStream(sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

        boost::property_tree::ini_parser::write_ini(writeStream, ptsm);

        writeStream.swap_vector(*ptString);
        // LOG(debug) << "CKeyValueGuard::putValue file content |" << *ptString << "|";
        // LOG(debug) << "Finish putValue key=" << _key << " value=" << _value;
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(fatal) << "key-value guard putValues exception: " << _e.what();
    }
    // }
    // LOG(debug) << "CKeyValueGuard::putValue key=" << _key << " value=" << _value << " done";
}

void CKeyValueGuard::getValue(const std::string& _key, std::string* _value, const std::string& _taskId)
{
    LOG(debug) << "CKeyValueGuard::getValue key=" << _key << " taskId=" << _taskId << " ...";
    if (_value == nullptr)
        throw invalid_argument("CKeyValueGuard::getValue: Value can't be NULL");

    const string sKey = _key + "." + _taskId;
    // TODO: Check if need this scope? Probably only for scoped_lock!
    // IMPORTANT: adding additional scope to make sure that the stream has flushed the data
    try
    {
        boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*m_sharedMemoryMutex);

        sharedMemoryString_t* ptString = m_sharedMemory->find_or_construct<sharedMemoryString_t>("KeyValue")(
            sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

        sharedMemoryVectorStream_t readStream(sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

        readStream.swap_vector(*ptString);

        boost::property_tree::ptree ptsm;
        boost::property_tree::ini_parser::read_ini(readStream, ptsm);

        // Swap back vectors
        readStream.swap_vector(*ptString);

        ptsm.get(sKey, *_value);
    }
    catch (boost::interprocess::interprocess_exception& ex)
    {
        LOG(error) << "Exception: " << ex.what();
    }
    catch (exception& ex)
    {
        LOG(error) << "Exception: " << ex.what();
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

    try
    {
        boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*m_sharedMemoryMutex);

        sharedMemoryString_t* ptString = m_sharedMemory->find_or_construct<sharedMemoryString_t>("KeyValue")(
            sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

        sharedMemoryVectorStream_t readStream(sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

        readStream.swap_vector(*ptString);

        boost::property_tree::ini_parser::read_ini(readStream, pt);

        // Swap back vectors
        readStream.swap_vector(*ptString);
    }
    catch (boost::interprocess::interprocess_exception& ex)
    {
        LOG(error) << "Exception: " << ex.what();
    }
    catch (exception& ex)
    {
        LOG(error) << "Exception: " << ex.what();
    }

    try
    {
        for (auto& element : pt.get_child(_key))
        {
            _values->insert(make_pair(element.first, element.second.get_value<std::string>()));
        }
    }
    catch (exception& ex)
    {
        LOG(error) << "key=" << _key << ": " << ex.what();
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

        try
        {
            m_agentConnectionMng->start();
        }
        catch (exception& _e)
        {
            LOG(fatal) << "AgentConnectionManager: exception in the transport service: " << _e.what();
        }
    }
}

int CKeyValueGuard::updateKey(const SUpdateKeyCmd& _cmd)
{
    if (!m_agentConnectionMng)
    {
        LOG(error) << "CKeyValueGuard::updateKey: Agent connection channel is not running. Failed to update " << _cmd;
        return 1;
    }

    LOG(debug) << "CKeyValueGuard::updateKey: sending key update: " << _cmd;
    return m_agentConnectionMng->updateKey(_cmd);
}

void CKeyValueGuard::deleteKey(const std::string& _key)
{
    LOG(debug) << "CKeyValueGuard::deleteKey key=" << _key;
    const string sKey = _key;
    {
        try
        {
            // TODO: Refactor this code because it is used now in three different places putValue, getValue, deleteKey.

            ip::scoped_lock<ip::named_mutex> lock(*m_sharedMemoryMutex);

            sharedMemoryString_t* ptString = m_sharedMemory->find_or_construct<sharedMemoryString_t>("KeyValue")(
                sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

            sharedMemoryVectorStream_t readStream(sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

            readStream.swap_vector(*ptString);

            boost::property_tree::ptree ptsm;
            boost::property_tree::ini_parser::read_ini(readStream, ptsm);

            ptsm.erase(sKey);

            sharedMemoryVectorStream_t writeStream(sharedMemoryCharAllocator_t(m_sharedMemory->get_segment_manager()));

            boost::property_tree::ini_parser::write_ini(writeStream, ptsm);

            writeStream.swap_vector(*ptString);
        }
        catch (const ip::interprocess_exception& _e)
        {
            LOG(fatal) << "key-value guard deleteKey exception: " << _e.what();
        }
    }
}
