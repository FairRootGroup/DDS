// DDS
#include "DDS/dds_intercom.h"
#include "DDS/dds_env_prop.h"
#include "DDS/Topology.h"

// STD
#include <exception>
#include <iostream>

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
using namespace dds::topology_api;

int main(int argc, char* argv[])
{
    try
    {
        // DDS Intercom API
        CIntercomService service;
        CCustomCmd customCmd(service);
        CKeyValue keyValue(service);
        
        service.subscribeOnError([](const EErrorCode _errorCode, const string& _errorMsg) {});
        
        customCmd.subscribe([](const string& _command, const string& _condition, uint64_t _senderId) {});
        customCmd.subscribeOnReply([](const string& _msg) { });
        
        keyValue.subscribe([](const string& _propertyName, const string& _value, uint64_t _senderTaskID) {});

        service.start();
        
        // DDS Topology API
        CTopology topology;
        
        // DDS env_prop
        std::string envSessionID(dds::env_prop<dds::EEnvProp::dds_session_id>());
        std::string envLocation(dds::env_prop<dds::EEnvProp::dds_location>());
        uint64_t envTaskID(dds::env_prop<dds::EEnvProp::task_id>());
        size_t envTaskIndex(dds::env_prop<dds::EEnvProp::task_index>());
        std::string envTaskName(dds::env_prop<dds::EEnvProp::task_name>());
        std::string envTaskPath(dds::env_prop<dds::EEnvProp::task_path>());
        size_t envCollectionIndex(dds::env_prop<dds::EEnvProp::collection_index>());
        std::string envCollectionName(dds::env_prop<dds::EEnvProp::collection_name>());
        std::string envGroupName(dds::env_prop<dds::EEnvProp::group_name>());
        
        cout << "Test project done successfully" << endl;
    }
    catch (exception& _e)
    {
        cerr << "Test project error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
