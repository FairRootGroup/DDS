//
//  DDSTopology.h
//  DDS
//
//  Created by Andrey Lebedev on 2/12/14.
//
//

// DDS
#include "DDSTopology.h"
// STL
#include <map>
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using boost::property_tree::ptree_bad_path;
using boost::property_tree::ptree_bad_data;
using boost::property_tree::ptree_error;
using boost::property_tree::ptree;
using boost::property_tree::xml_parser_error;

using namespace std;

DDSTopology::DDSTopology()
    : m_tasks()
    , m_collections()
    , m_groups()
{
}

DDSTopology::~DDSTopology()
{
}

void DDSTopology::init(const string& _fileName)
{
    ptree pt;

    try
    {
        read_xml(_fileName, pt);
    }
    catch (xml_parser_error& error)
    {
        // FIXME: What to do in case of fail?
        cout << error.what();
    }

    PrintPropertyTree("", pt);

    ParsePropertyTree(pt);

    cout << toString();
}

void DDSTopology::ParsePropertyTree(const ptree& _pt)
{
    try
    {
        const ptree& pt = _pt.get_child("topology");

        for (const auto& v : pt)
        {
            if (v.first == "port")
                ParsePort(v.second);
            else if (v.first == "task")
                ParseTask(v.second);
            else if (v.first == "collection")
                ParseTaskCollection(v.second);
            else if (v.first == "group")
                ParseTaskGroup(v.second);
            else if (v.first == "main")
                ParseMain(v.second);
        }
    }
    catch (ptree_bad_path& error)
    {
        cout << "ptree_bad_path: " << error.what() << endl;
    }
    catch (ptree_bad_data& error)
    {
        cout << "ptree_bad_data: " << error.what() << endl;
    }
    catch (ptree_error& error)
    {
        cout << "ptree_error: " << error.what() << endl;
    }
    catch (out_of_range& error)
    {
        cout << "out_of_range: " << error.what() << endl;
    }
}

void DDSTopology::ParsePort(const ptree& _pt)
{
    string name = _pt.get<string>("<xmlattr>.name");
    unsigned int min = _pt.get<unsigned int>("<xmlattr>.min");
    unsigned int max = _pt.get<unsigned int>("<xmlattr>.max");

    DDSPortPtr_t newPort = make_shared<DDSPort>();
    newPort->setName(name);
    newPort->setRange(min, max);

    m_ports[name] = newPort;
}

void DDSTopology::ParseTask(const ptree& _pt)
{
    string name = _pt.get<string>("<xmlattr>.name");
    string exec = _pt.get<string>("<xmlattr>.exec");

    DDSTaskPtr_t newTask = make_shared<DDSTask>();
    newTask->setName(name);
    newTask->setExec(exec);

    for (const auto& v : _pt)
    {
        if (v.first == "port")
        {
            string portName = v.second.get<string>("<xmlattr>.name");
            // Check if port exists
            auto port = m_ports.find(name);
            if (port != m_ports.end())
            {
                newTask->addPort(port->second);
            }
            else
            {
                throw out_of_range(name + " port does not exist.");
            }
        }
    }

    m_tasks[name] = newTask;
}

void DDSTopology::ParseTaskCollection(const ptree& _pt)
{
    string name = _pt.get<string>("<xmlattr>.name");

    DDSTaskCollectionPtr_t newTaskCollection = make_shared<DDSTaskCollection>();
    newTaskCollection->setName(name);

    for (const auto& v : _pt)
    {
        if (v.first == "task")
        {
            string name = v.second.get<string>("<xmlattr>.name");
            // Check if task exists
            auto task = m_tasks.find(name);
            if (task != m_tasks.end())
            {
                newTaskCollection->addTask(task->second);
            }
            else
            {
                throw out_of_range(name + " task does not exist.");
            }
        }
    }

    m_collections[name] = newTaskCollection;
}

void DDSTopology::ParseTaskGroup(const ptree& _pt)
{
    string name = _pt.get<string>("<xmlattr>.name");

    DDSTaskGroupPtr_t newTaskGroup = make_shared<DDSTaskGroup>();
    newTaskGroup->setName(name);

    for (const auto& v : _pt)
    {
        if (v.first == "task")
        {
            string name = v.second.get<string>("<xmlattr>.name");
            // Check if task exists
            auto task = m_tasks.find(name);
            if (task != m_tasks.end())
            {
                newTaskGroup->addTask(task->second);
            }
            else
            {
                throw out_of_range(name + " task does not exist.");
            }
        }
        else if (v.first == "collection")
        {
            string name = v.second.get<string>("<xmlattr>.name");
            // Check if task collection exists
            auto collection = m_collections.find(name);
            if (collection != m_collections.end())
            {
                newTaskGroup->addTaskCollection(collection->second);
            }
            else
            {
                throw out_of_range(name + " task collection does not exist.");
            }
        }
    }

    m_groups[name] = newTaskGroup;
}

void DDSTopology::ParseMain(const boost::property_tree::ptree& _pt)
{
    //   string name = v.second.get<string>("group.<xmlattr>.name");
    //   size_t n = v.second.get<size_t>("group.<xmlattr>.n");
}

void DDSTopology::PrintPropertyTree(const string& _path, const ptree& _pt) const
{
    if (_pt.size() == 0)
    {
        cout << _path << " " << _pt.get_value("") << endl;
        return;
    }
    for (const auto& v : _pt)
    {
        string path = (_path != "") ? (_path + "." + v.first) : v.first;
        PrintPropertyTree(path, v.second);
    }
}
