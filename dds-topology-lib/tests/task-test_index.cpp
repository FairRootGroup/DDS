// DDS
#include "Logger.h"
#include "KeyValue.h"
// STD
#include <exception>
#include <sstream>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace dds;
namespace bpo = boost::program_options;
using namespace MiscCommon;

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

        CKeyValue ddsKeyValue;

        // Get environment variables
        const char* ddsTaskId = getenv("DDS_TASK_ID");
        string taskId;
        if (NULL == ddsTaskId)
        {
            LOG(log_stderr) << "USER TASK: DDS_TASK_ID variable is not set.";
        }
        else
        {
            taskId = ddsTaskId;
        }

        const char* ddsTaskIndex = getenv("DDS_TASK_INDEX");
        string taskIndex("NOT SET");
        if (NULL == ddsTaskIndex)
        {
            LOG(log_stderr) << "USER TASK: DDS_TASK_INDEX variable is not set.";
        }
        else
        {
            taskIndex = ddsTaskIndex;
        }

        const char* ddsCollectionIndex = getenv("DDS_COLLECTION_INDEX");
        string collectionIndex("NOT SET");
        if (NULL == ddsCollectionIndex)
        {
            LOG(log_stderr) << "USER TASK: DDS_COLLECTION_INDEX variable is not set";
        }
        else
        {
            collectionIndex = ddsCollectionIndex;
        }

        stringstream ss;
        ss << "DDS_TASK_ID=" << taskId << " optType=" << optType << " optTaskIndex=" << optTaskIndex
           << " optCollectionIndex=" << optCollectionIndex << " DDS_TASK_INDEX=" << taskIndex
           << " DDS_COLLECTION_INDEX=" << collectionIndex;
        ddsKeyValue.putValue("IndexInfo", ss.str());

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
