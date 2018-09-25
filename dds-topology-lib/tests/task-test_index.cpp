// DDS
#include "dds_env_prop.h"
#include "dds_intercom.h"
// STD
#include <exception>
#include <iostream>
#include <sstream>
// BOOST
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
namespace bpo = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char* argv[])
{
    try
    {
        int optType;
        string optTaskIndex;
        string optCollectionIndex;

        // Generic options
        bpo::options_description options("task-test_index options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("type", bpo::value<int>(&optType), "type of user task");
        options.add_options()("taskIndex", bpo::value<string>(&optTaskIndex), "task index");
        options.add_options()("collectionIndex", bpo::value<string>(&optCollectionIndex), "collection index");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            cout << options;
            return 0;
        }

        // Environment variables set by DDS
        std::string envSessionID(dds::env_prop<dds::EEnvProp::dds_session_id>());
        std::string envLocation(dds::env_prop<dds::EEnvProp::dds_location>());
        uint64_t envTaskID(dds::env_prop<dds::EEnvProp::task_id>());
        size_t envTaskIndex(dds::env_prop<dds::EEnvProp::task_index>());
        std::string envTaskName(dds::env_prop<dds::EEnvProp::task_name>());
        std::string envTaskPath(dds::env_prop<dds::EEnvProp::task_path>());
        size_t envCollectionIndex(dds::env_prop<dds::EEnvProp::collection_index>());
        std::string envCollectionName(dds::env_prop<dds::EEnvProp::collection_name>());
        std::string envGroupName(dds::env_prop<dds::EEnvProp::group_name>());

        bool optIndexOK(true);
        size_t optTaskIndexInt(0);
        size_t optCollectionIndexInt(0);
        try
        {
            optTaskIndexInt = boost::lexical_cast<size_t>(optTaskIndex);
            optCollectionIndexInt = boost::lexical_cast<size_t>(optCollectionIndex);
        }
        catch (std::exception& _e)
        {
            optIndexOK = false;
            optTaskIndexInt = 0;
            optCollectionIndexInt = 0;
            cerr << "USER TASK Error: Opt collection or task index is wrong; optCollectionIndex=" << optCollectionIndex
                 << "; optTaskIndex=" << optTaskIndex << endl;
        }

        // Check environment variables set by DDS
        bool indexOK = (optIndexOK && optCollectionIndexInt == envCollectionIndex && optTaskIndexInt == envTaskIndex) ||
                       !optIndexOK;
        if (!indexOK)
        {
            cerr << "USER TASK Error: Collection or task index is wrong; optCollectionIndexInt="
                 << optCollectionIndexInt << "; envCollectionIndex=" << envCollectionIndex
                 << "; optTaskIndexInt=" << optTaskIndexInt << "; envTaskIndex=" << envTaskIndex << endl;
        }

        // envCollectionName can also be empty
        bool namesOK = envTaskName.size() > 0 && envGroupName.size() > 0;
        if (!namesOK)
        {
            cerr << "USER TASK Error: Task, collection or group name is wrong; envTaskName=" << envTaskName
                 << "; envCollectionName=" << envCollectionName << "; envGroupName=" << envGroupName << endl;
        }

        bool taskPathOK = envTaskPath.size() > 0;
        if (!taskPathOK)
        {
            cerr << "USER TASK Error: Task path is wrong; envTaskPath=" << envTaskPath << endl;
        }

        bool locationOK = fs::is_directory(fs::path(envLocation));
        if (!locationOK)
        {
            cerr << "USER TASK Error: DDS location is wrong; envLocation=" << envLocation << endl;
        }

        bool sessionIDOK(true);
        try
        {
            boost::uuids::uuid sessionID = boost::uuids::string_generator()(envSessionID);
            if (sessionID.version() == boost::uuids::uuid::version_unknown)
                throw std::logic_error("");
        }
        catch (std::exception& _e)
        {
            sessionIDOK = false;
            cerr << "USER TASK Error: DDS Session ID is wrong; envSessionID=" << envSessionID << endl;
        }

        bool taskIDOK(envTaskID > 0);

        bool envTestPassed = indexOK && namesOK && taskPathOK && locationOK && sessionIDOK && taskIDOK;

        // Check exception handling in key-value and custom commands
        CIntercomService service;
        CKeyValue keyValue(service);
        CCustomCmd customCmd(service);

        // if type == 0 no exception is thrown;
        // if type == 1 exception is thrown only from subscribe;
        // if type == 2 exception is thrown from both subscribe and subscribeOnError;

        int callbackCounter = 0;

        service.subscribeOnError([optType, &callbackCounter](EErrorCode _errorCode, const string& _msg) {
            callbackCounter++;
            cerr << "DDS key-value error code: " << _errorCode << ", message: " << _msg << endl;
            if (optType == 2)
            {
                throw std::runtime_error("Exception in subscribeOnError");
            }
        });

        keyValue.subscribe(
            [optType, &callbackCounter](const string& _propertyID, const string& _value, uint64_t _senderTaskID) {
                callbackCounter++;
                cout << "Received key-value update: propertyID=" << _propertyID << " value=" << _value
                     << " senderTaskID=" << _senderTaskID << std::endl;
                if (optType == 1 || optType == 2)
                {
                    throw std::runtime_error("Exception in keyValue.subscribe");
                }
            });

        keyValue.subscribeOnDelete([optType, &callbackCounter](const string& _propertyID, const string& _key) {
            callbackCounter++;
            cout << "Delete key notification received for key " << _key;
            if (optType == 1 || optType == 2)
            {
                throw std::runtime_error("Exception in keyValue.subscribeOnDelete");
            }
        });

        customCmd.subscribe(
            [optType, &callbackCounter](const string& _command, const string& _condition, uint64_t _senderId) {
                callbackCounter++;
                cout << "Received custom command: " << _command << " condition: " << _condition
                     << " senderId: " << _senderId << endl;
                if (optType == 1 || optType == 2)
                {
                    throw std::runtime_error("Exception in customCmd.subscribe");
                }
            });

        customCmd.subscribeOnReply([optType, &callbackCounter](const string& _msg) {
            callbackCounter++;
            cout << "Received reply message: " << _msg << endl;
            if (optType == 1 || optType == 2)
            {
                throw std::runtime_error("Exception in customCmd.subscribeOnReply");
            }
        });

        service.start();

        if (optType == 0)
        {
            keyValue.putValue("TestKey", "TestValue");
            customCmd.send("TestCommand", "");
        }

        sleep(10);

        cout << "USER TASK optType=" << optType << "; callbackCounter=" << callbackCounter << endl;

        bool intercomTestPassed(true);
        if (optType == 0)
        {
            intercomTestPassed = callbackCounter == 1;
        }
        else if (optType == 1)
        {
            intercomTestPassed = callbackCounter == 4;
        }
        else if (optType == 2)
        {
            intercomTestPassed = callbackCounter == 4;
        }

        if (envTestPassed && intercomTestPassed)
        {
            cout << "USER TASK test passed" << endl;
        }
        else
        {
            cout << "USER TASK test failed" << endl;
        }

        cout << "USER TASK done" << endl;
    }
    catch (const exception& _e)
    {
        cerr << "USER TASK Error: " << _e.what() << endl;
        return 1;
    }
    return 0;
}
