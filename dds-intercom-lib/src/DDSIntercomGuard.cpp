// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "DDSIntercomGuard.h"
#include "BOOST_FILESYSTEM.h"
#include "UserDefaults.h"

// BOOST
#include <boost/property_tree/ptree.hpp>

// silence "Unused typedef" warning using clang 3.7+ and boost < 1.59
#if BOOST_VERSION < 105900
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#include <boost/property_tree/ini_parser.hpp>
#if BOOST_VERSION < 105900
#pragma clang diagnostic pop
#endif

using namespace std;
using namespace dds;
using namespace dds::user_defaults_api;
using namespace dds::internal_api;
using namespace dds::protocol_api;
using namespace MiscCommon;
namespace ip = boost::interprocess;

CDDSIntercomGuard::CDDSIntercomGuard()
{
    Logger::instance().init(); // Initialize log

    boost::filesystem::path cfgFile(CUserDefaults::instance().getDDSPath());
    cfgFile /= "task.cfg";
    m_sCfgFilePath = cfgFile.generic_string();
}

CDDSIntercomGuard::~CDDSIntercomGuard()
{
}

CDDSIntercomGuard& CDDSIntercomGuard::instance()
{
    static CDDSIntercomGuard instance;
    return instance;
}

connection_t CDDSIntercomGuard::connectError(intercom_api::errorSignal_t::slot_function_type _subscriber)
{
    return m_syncHelper.m_errorSignal.connect(_subscriber);
}

connection_t CDDSIntercomGuard::connectCustomCmd(customCmdSignal_t::slot_function_type _subscriber)
{
    return m_syncHelper.m_customCmdSignal.connect(_subscriber);
}

connection_t CDDSIntercomGuard::connectCustomCmdReply(customCmdReplySignal_t::slot_function_type _subscriber)
{
    return m_syncHelper.m_customCmdReplySignal.connect(_subscriber);
}

connection_t CDDSIntercomGuard::connectKeyValue(keyValueSignal_t::slot_function_type _subscriber)
{
    return m_syncHelper.m_keyValueUpdateSig.connect(_subscriber);
}

void CDDSIntercomGuard::disconnectCustomCmd()
{
    // disconnect custom command signals
    m_syncHelper.m_customCmdSignal.disconnect_all_slots();
    m_syncHelper.m_customCmdReplySignal.disconnect_all_slots();
    m_syncHelper.m_errorSignal.disconnect_all_slots();
}

void CDDSIntercomGuard::disconnectKeyValue()
{
    // disconnect key-value signals
    m_syncHelper.m_keyValueUpdateSig.disconnect_all_slots();
    m_syncHelper.m_errorSignal.disconnect_all_slots();
}

void CDDSIntercomGuard::initAgentConnection()
{
    lock_guard<std::mutex> lock(m_initAgentConnectionMutex);

    LOG(info) << "CCDDSIntercomGuard::initAgentConnection: is going to init";

    if (!m_agentConnectionMng)
    {
        LOG(info) << "CCDDSIntercomGuard::initAgentConnection: start init";

        m_agentConnectionMng.reset();
        m_agentConnectionMng = make_shared<CAgentConnectionManager>();
        m_agentConnectionMng->m_syncHelper = &m_syncHelper;
        m_agentConnectionMng->start();
    }
}

int CDDSIntercomGuard::sendCustomCmd(const protocol_api::SCustomCmdCmd& _command)
{
    if (!m_agentConnectionMng)
    {
        stringstream ss;
        ss << "Agent connection channel is not running. Failed to send custom command: " << _command;
        LOG(error) << ss.str();
        m_syncHelper.m_errorSignal(intercom_api::EErrorCode::SendCustomCmdFailed, ss.str());
        return 1;
    }

    LOG(info) << "CCDDSIntercomGuard::sendCmd: sending custom command: " << _command;
    return m_agentConnectionMng->sendCustomCmd(_command);
}

void CDDSIntercomGuard::createStorage()
{
    // Create shared memory storage and semaphor to synchronize accesss to shared memory

    string storageName(to_string(CUserDefaults::instance().getScoutPid()));

    // Shared memory storage for property tree
    string sharedMemoryName(storageName);
    sharedMemoryName += "_DDSSM";

    const bool sharedMemoryRemoved = ip::shared_memory_object::remove(sharedMemoryName.c_str());
    LOG(info) << "Shared memory " << sharedMemoryName << " remove status: " << sharedMemoryRemoved;

    m_sharedMemory.reset();
    try
    {
        LOG(info) << "Creating key-value shared memory with name " << sharedMemoryName;
        // boost::interprocess::permissions perm;
        // perm.set_unrestricted();
        m_sharedMemory = make_shared<ip::managed_shared_memory>(
            ip::open_or_create, sharedMemoryName.c_str(), 1024 * 1024); //, nullptr, perm);
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(fatal) << "Can't initialize key-value shared memory with name " << sharedMemoryName << ": " << _e.what();
    }

    // Named mutex
    string mutexName(storageName);
    mutexName += "_DDSM";

    const bool removed = ip::named_mutex::remove(mutexName.c_str());
    LOG(info) << "Named mutex remove status: " << removed;

    m_sharedMemoryMutex.reset();
    try
    {
        LOG(info) << "Creating key-value lock object with name " << mutexName;
        // boost::interprocess::permissions perm;
        // perm.set_unrestricted();
        m_sharedMemoryMutex = make_shared<ip::named_mutex>(ip::open_or_create, mutexName.c_str()); //, perm);
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(fatal) << "Can't initialize key-value lock object with name " << mutexName << ": " << _e.what();
    }
}

void CDDSIntercomGuard::initLock()
{
    if (m_sharedMemoryMutex)
        return;

    string storageName(to_string(CUserDefaults::instance().getScoutPid()));

    // Shared memory storage for property tree
    string sharedMemoryName(storageName);
    sharedMemoryName += "_DDSSM";

    try
    {
        LOG(info) << "Openning key-value shared memory with name " << sharedMemoryName;
        m_sharedMemory = make_shared<ip::managed_shared_memory>(ip::open_only, sharedMemoryName.c_str());
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(fatal) << "Can't open key-value shared memory with name " << sharedMemoryName << ": " << _e.what();
    }

    // Named mutex
    string mutexName(storageName);
    mutexName += "_DDSM";

    try
    {
        LOG(info) << "Openning key-value lock object with name " << mutexName;
        m_sharedMemoryMutex = make_shared<ip::named_mutex>(ip::open_only, mutexName.c_str());
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(fatal) << "Can't open key-value lock object with name " << mutexName << ": " << _e.what();
    }
}

void CDDSIntercomGuard::putValue(const std::string& _key, const std::string& _value, const std::string& _taskId)
{
    const string sKey = _key + "." + _taskId;
    putValue(sKey, _value);
}

void CDDSIntercomGuard::putValue(const std::string& _key, const std::string& _value)
{
    LOG(debug) << "CCDDSIntercomGuard::putValue key=" << _key << " value=" << _value << " ...";
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
            // LOG(debug) << "CCDDSIntercomGuard::putValue file content |" << *ptString << "|";
            LOG(debug) << "Finish putValue key=" << _key << " value=" << _value;
        }
        catch (const ip::interprocess_exception& _e)
        {
            LOG(fatal) << "key-value guard putValue exception: " << _e.what();
        }
    }
    LOG(debug) << "CCDDSIntercomGuard::putValue key=" << _key << " value=" << _value << " done";
}

void CDDSIntercomGuard::putValues(const std::vector<SCommandAttachmentImpl<cmdUPDATE_KEY>::ptr_t>& _values)
{
    // LOG(debug) << "CCDDSIntercomGuard::putValue key=" << _key << " value=" << _value << " ...";
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
        // LOG(debug) << "CCDDSIntercomGuard::putValue file content |" << *ptString << "|";
        // LOG(debug) << "Finish putValue key=" << _key << " value=" << _value;
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(fatal) << "key-value guard putValues exception: " << _e.what();
    }
    // }
    // LOG(debug) << "CCDDSIntercomGuard::putValue key=" << _key << " value=" << _value << " done";
}

void CDDSIntercomGuard::getValue(const std::string& _key, std::string* _value, const std::string& _taskId)
{
    LOG(debug) << "CCDDSIntercomGuard::getValue key=" << _key << " taskId=" << _taskId << " ...";
    if (_value == nullptr)
        throw invalid_argument("CCDDSIntercomGuard::getValue: Value can't be NULL");

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
    LOG(debug) << "CCDDSIntercomGuard::getValue key=" << _key << " taskId=" << _taskId << " done";
}

void CDDSIntercomGuard::getValues(const std::string& _key, valuesMap_t* _values)
{
    LOG(debug) << "CCDDSIntercomGuard::getValues key=" << _key << " ...";

    if (!m_sharedMemoryMutex)
        throw invalid_argument(
            "CCDDSIntercomGuard::getValue: can't lock shared memmory. Probably DDS agent is offline.");

    if (_values == nullptr)
        throw invalid_argument("CCDDSIntercomGuard::getValue: _values can't be NULL");

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
        LOG(warning) << "key=" << _key << ": " << ex.what();
        return;
    }
    LOG(debug) << "CCDDSIntercomGuard::getValues key=" << _key << " done";
}

int CDDSIntercomGuard::updateKey(const SUpdateKeyCmd& _cmd)
{
    if (!m_agentConnectionMng)
    {
        stringstream ss;
        ss << "Agent connection channel is not running. Failed to update key-value: " << _cmd;
        LOG(error) << ss.str();
        m_syncHelper.m_errorSignal(intercom_api::EErrorCode::SendKeyValueFailed, ss.str());
        return 1;
    }

    LOG(info) << "CCDDSIntercomGuard::updateKey: sending key update: " << _cmd;
    return m_agentConnectionMng->updateKey(_cmd);
}

void CDDSIntercomGuard::deleteKey(const std::string& _key)
{
    LOG(debug) << "CCDDSIntercomGuard::deleteKey key=" << _key;
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

void CDDSIntercomGuard::clean()
{
    string storageName(to_string(CUserDefaults::instance().getScoutPid()));

    string sharedMemoryName(storageName);
    sharedMemoryName += "_DDSSM";

    string mutexName(storageName);
    mutexName += "_DDSM";

    try
    {
        const bool sharedMemoryRemoved = ip::shared_memory_object::remove(sharedMemoryName.c_str());
        LOG(log_stdout) << "Shared memory " << sharedMemoryName << " remove status: " << sharedMemoryRemoved;

        const bool mutexRemoved = ip::named_mutex::remove(mutexName.c_str());
        LOG(log_stdout) << "Shared memory " << mutexName << " remove status: " << mutexRemoved;
    }
    catch (const ip::interprocess_exception& _e)
    {
        LOG(log_stderr) << "Can't remove shared memory " << sharedMemoryName << " and shared mutex " << mutexName
                        << ": " << _e.what();
    }
}

void CDDSIntercomGuard::waitCondition()
{
    if (!m_agentConnectionMng)
    {
        LOG(error) << "CCDDSIntercomGuard::waitCondition: Agent connection channel is not running.";
        return;
    }

    LOG(info) << "CCDDSIntercomGuard::waitCondition: stop thread and wait for notification.";
    return m_agentConnectionMng->waitCondition();
}

void CDDSIntercomGuard::stopCondition()
{
    if (!m_agentConnectionMng)
    {
        LOG(error) << "CCDDSIntercomGuard::stopCondition: Agent connection channel is not running.";
        return;
    }

    LOG(info) << "CCDDSIntercomGuard::stopCondition: notification received, continue execution.";
    return m_agentConnectionMng->stopCondition();
}
