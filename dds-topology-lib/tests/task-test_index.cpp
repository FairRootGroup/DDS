// DDS
#include "Logger.h"
#include "dds_intercom.h"
// STD
#include <exception>
#include <sstream>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace dds;
using namespace dds::intercom_api;
namespace bpo = boost::program_options;
using namespace MiscCommon;

void setVarFromEnv(const string& _name, string& _value)
{
    // DDS_GROUP_NAME
    const char* ddsVar = getenv(_name.c_str());
    _value = "NOT SET";
    if (NULL == ddsVar)
    {
        LOG(log_stderr) << "USER TASK: $" << _name << " variable is not set";
    }
    else
    {
        _value = ddsVar;
    }
}

int main(int argc, char* argv[])
{
    try
    {
        Logger::instance().init(); // Initialize log
        // dds::CUserDefaults::instance(); // Initialize user defaults

        string optType;
        string optTaskIndex;
        string optCollectionIndex;

        // Generic options
        bpo::options_description options("task-test_index options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("type", bpo::value<string>(&optType), "type of user task");
        options.add_options()("taskIndex", bpo::value<string>(&optTaskIndex), "task index");
        options.add_options()("collectionIndex", bpo::value<string>(&optCollectionIndex), "collection index");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            cout << options;
            return false;
        }

        CKeyValue keyValue;

        // Get environment variables
        // DDS_TASK_ID
        string taskId;
        setVarFromEnv("DDS_TASK_ID", taskId);

        // DDS_TASK_INDEX
        string taskIndex;
        setVarFromEnv("DDS_TASK_INDEX", taskIndex);

        // DDS_COLLECTION_INDEX
        string collectionIndex;
        setVarFromEnv("DDS_COLLECTION_INDEX", collectionIndex);

        // DDS_TASK_PATH
        string taskPath;
        setVarFromEnv("DDS_TASK_PATH", taskPath);

        // DDS_GROUP_NAME
        string groupName;
        setVarFromEnv("DDS_GROUP_NAME", groupName);

        // DDS_COLLECTION_NAME
        string collectionName;
        setVarFromEnv("DDS_COLLECTION_NAME", collectionName);

        // DDS_TASK_NAME
        string taskName;
        setVarFromEnv("DDS_TASK_NAME", taskName);

        stringstream ss;
        ss << "DDS_TASK_ID=" << taskId << " optType=" << optType << " optTaskIndex=" << optTaskIndex
           << " optCollectionIndex=" << optCollectionIndex << " DDS_TASK_INDEX=" << taskIndex
           << " DDS_COLLECTION_INDEX=" << collectionIndex << " DDS_TASK_PATH=" << taskPath
           << " DDS_GROUP_NAME=" << groupName << " DDS_COLLECTION_NAME=" << collectionName
           << " DDS_TASK_NAME=" << taskName;
        keyValue.putValue("IndexInfo", ss.str());

        LOG(log_stdout) << "USER TASK: " << ss.str();

        sleep(60);
    }
    catch (const exception& _e)
    {
        LOG(log_stderr) << "USER TASK Error: " << _e.what() << endl;
        return 1;
    }
    return 0;
}
