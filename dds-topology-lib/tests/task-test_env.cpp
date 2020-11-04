// DDS
#include "EnvProp.h"
#include "Logger.h"
// STD
#include <exception>
// BOOST
#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace std;
using namespace dds;
using namespace dds::user_defaults_api;
using namespace MiscCommon;
namespace bpo = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char* argv[])
{
    try
    {
        CUserDefaults::instance(); // Initialize user defaults
        Logger::instance().init(); // Initialize log

        size_t optTaskIndex;
        string optCollectionIndexStr;

        // Generic options
        bpo::options_description options("task-test_index options");
        options.add_options()("taskIndex", bpo::value<size_t>(&optTaskIndex), "Task index");
        options.add_options()("collectionIndex", bpo::value<string>(&optCollectionIndexStr), "Collection index");

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(options).run(), vm);
        bpo::notify(vm);

        // Environment variables set by DDS
        const string envSessionID(env_prop<EEnvProp::dds_session_id>());
        const string envLocation(env_prop<EEnvProp::dds_location>());
        const uint64_t envTaskID(env_prop<EEnvProp::task_id>());
        const size_t envTaskIndex(env_prop<EEnvProp::task_index>());
        const string envTaskName(env_prop<EEnvProp::task_name>());
        const string envTaskPath(env_prop<EEnvProp::task_path>());
        const size_t envCollectionIndex(env_prop<EEnvProp::collection_index>());
        const string envCollectionName(env_prop<EEnvProp::collection_name>());
        const string envGroupName(env_prop<EEnvProp::group_name>());
        const uint64_t envSlotID(env_prop<EEnvProp::dds_slot_id>());

        //
        // Basic check of environment variables set by DDS
        //

        // Collection index is not set for tasks outside the collection
        bool collectionIndexOk{ true };
        try
        {
            size_t optCollectionIndex{ boost::lexical_cast<size_t>(optCollectionIndexStr) };
            collectionIndexOk = optCollectionIndex == envCollectionIndex;
        }
        catch (exception& _e)
        {
            collectionIndexOk = optCollectionIndexStr == "%collectionIndex%";
        }

        const bool result{ !boost::uuids::string_generator()(envSessionID).is_nil() &&
                           fs::is_directory(fs::path(envLocation)) && envTaskID > 0 && optTaskIndex == envTaskIndex &&
                           envTaskName.size() > 0 && envTaskPath.size() > 0 && collectionIndexOk &&
                           envGroupName.size() > 0 && envSlotID > 0 };

        if (result)
        {
            LOG(info) << "Task successfully done";
            return EXIT_SUCCESS;
        }
        else
        {
            LOG(error) << "Task failed";
            return EXIT_FAILURE;
        }
    }
    catch (const exception& _e)
    {
        cerr << "Error: " << _e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
