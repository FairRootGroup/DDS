// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopology.h"
// STL
#include <map>
// SYSTEM
#include <unistd.h>
#include <sys/wait.h>
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
    : m_tempPorts()
    , m_tempTasks()
    , m_tempCollections()
    , m_tempGroups()
    , m_main()
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

    //    PrintPropertyTree("", pt);

    ParsePropertyTree(pt);

    // cout << toString();
}

bool DDSTopology::isValid(const std::string& _fileName)
{
    pid_t pid = fork();

    switch (pid)
    {
        case -1:
            // Unable to fork
            throw runtime_error("Unable to run XML validator.");
        case 0:
        {
            // FIXME: XSD file is hardcoded now -> take it from resource manager
            // FIXME: Path to xmllint is hardcoded now.
            execl("/usr/bin/xmllint", "xmllint", "--noout", "--schema", "topology.xsd", _fileName.c_str(), NULL);

            // We shoud never come to this point of execution
            exit(1);
        }
    }

    int status = -1;
    while (wait(&status) != pid)
        ;

    //    do
    //    {
    //        pid_t w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
    //        if (w == -1)
    //        {
    //            perror("waitpid");
    //            exit(EXIT_FAILURE);
    //        }
    //
    //        if (WIFEXITED(status))
    //        {
    //            printf("exited, status=%d\n", WEXITSTATUS(status));
    //        }
    //        else if (WIFSIGNALED(status))
    //        {
    //            printf("killed by signal %d\n", WTERMSIG(status));
    //        }
    //        else if (WIFSTOPPED(status))
    //        {
    //            printf("stopped by signal %d\n", WSTOPSIG(status));
    //        }
    //        else if (WIFCONTINUED(status))
    //        {
    //            printf("continued\n");
    //        }
    //    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    // std::cout << "pid=" << pid << std::endl;
    // std::cout << "status=" << status << std::endl;

    return (status == 0);
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

        m_tempPorts.clear();
        m_tempTasks.clear();
        m_tempCollections.clear();
        m_tempGroups.clear();
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

    if (m_tempPorts.find(name) == m_tempPorts.end())
    {
        m_tempPorts[name] = newPort;
    }
    else
    {
        throw logic_error("Port " + name + " already exists");
    }
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
            auto port = m_tempPorts.find(portName);
            if (port != m_tempPorts.end())
            {
                newTask->addPort(*(port->second.get()));
            }
            else
            {
                throw out_of_range(portName + " port does not exist.");
            }
        }
    }

    if (m_tempTasks.find(name) == m_tempTasks.end())
    {
        m_tempTasks[name] = newTask;
    }
    else
    {
        throw logic_error("Task " + name + " already exists");
    }
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
            string taskName = v.second.get<string>("<xmlattr>.name");
            auto task = m_tempTasks.find(taskName);
            if (task != m_tempTasks.end())
            {
                newTaskCollection->addElement(dynamic_pointer_cast<DDSTopoElement>(task->second));
            }
            else
            {
                throw out_of_range(taskName + " task does not exist");
            }
        }
    }

    if (m_tempCollections.find(name) == m_tempCollections.end())
    {
        m_tempCollections[name] = newTaskCollection;
    }
    else
    {
        throw logic_error("Task collection " + name + " already exists");
    }
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
            string taskName = v.second.get<string>("<xmlattr>.name");
            auto task = m_tempTasks.find(taskName);
            if (task != m_tempTasks.end())
            {
                newTaskGroup->addElement(dynamic_pointer_cast<DDSTopoElement>(task->second));
            }
            else
            {
                throw out_of_range(taskName + " task does not exist.");
            }
        }
        else if (v.first == "collection")
        {
            string collectionName = v.second.get<string>("<xmlattr>.name");
            auto collection = m_tempCollections.find(collectionName);
            if (collection != m_tempCollections.end())
            {
                newTaskGroup->addElement(dynamic_pointer_cast<DDSTopoElement>(collection->second));
            }
            else
            {
                throw out_of_range(collectionName + " task collection does not exist.");
            }
        }
    }

    if (m_tempGroups.find(name) == m_tempGroups.end())
    {
        m_tempGroups[name] = newTaskGroup;
    }
    else
    {
        throw logic_error("Task group " + name + " already exists");
    }
}

void DDSTopology::ParseMain(const boost::property_tree::ptree& _pt)
{
    m_main = make_shared<DDSTaskGroup>();

    m_main->setN(_pt.get<size_t>("<xmlattr>.n"));
    m_main->setMinimumRequired(_pt.get<size_t>("<xmlattr>.minRequired"));

    for (const auto& v : _pt)
    {
        if (v.first == "task")
        {
            string name = v.second.get<string>("<xmlattr>.name");
            auto task = m_tempTasks.find(name);
            if (task != m_tempTasks.end())
            {
                DDSTaskPtr_t newTask = make_shared<DDSTask>();
                m_main->addElement(newTask);
            }
            else
            {
                throw out_of_range(name + " task does not exist.");
            }
        }
        else if (v.first == "collection")
        {
            string name = v.second.get<string>("<xmlattr>.name");
            auto collection = m_tempCollections.find(name);
            if (collection != m_tempCollections.end())
            {
                DDSTaskCollectionPtr_t newCollection = make_shared<DDSTaskCollection>();
                newCollection->setN(_pt.get<size_t>("<xmlattr>.n"));
                newCollection->setMinimumRequired(_pt.get<size_t>("<xmlattr>.minRequired"));
                m_main->addElement(newCollection);
            }
            else
            {
                throw out_of_range(name + " task collection does not exist.");
            }
        }
        else if (v.first == "group")
        {
            string name = v.second.get<string>("<xmlattr>.name");
            auto group = m_tempGroups.find(name);
            if (group != m_tempGroups.end())
            {
                DDSTaskGroupPtr_t newGroup = make_shared<DDSTaskGroup>();
                newGroup->setN(_pt.get<size_t>("<xmlattr>.n"));
                newGroup->setMinimumRequired(_pt.get<size_t>("<xmlattr>.minRequired"));
                m_main->addElement(newGroup);
            }
            else
            {
                throw out_of_range(name + " task group does not exist.");
            }
        }
    }
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
