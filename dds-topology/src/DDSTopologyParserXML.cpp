// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopologyParserXML.h"
#include "DDSPort.h"
// STL
#include <map>
// SYSTEM
#include <unistd.h>
#include <sys/wait.h>
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace boost::property_tree;
using namespace std;

DDSTopologyParserXML::DDSTopologyParserXML()
    : m_tempPorts()
    , m_tempTasks()
    , m_tempCollections()
    , m_tempGroups()
    , m_main()
{
}

DDSTopologyParserXML::~DDSTopologyParserXML()
{
}

bool DDSTopologyParserXML::isValid(const std::string& _fileName)
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

    return (status == 0);
}

DDSTaskGroupPtr_t DDSTopologyParserXML::parse(const string& _fileName)
{
    // First validate XML against XSD schema
    try
    {
        if (!isValid(_fileName))
            return nullptr;
    }
    catch (runtime_error& error)
    {
        cout << error.what();
        return nullptr;
    }

    ptree pt;

    // Read property tree from file
    try
    {
        read_xml(_fileName, pt);
    }
    catch (xml_parser_error& error)
    {
        cout << error.what();
        return nullptr;
    }

    //    PrintPropertyTree("", pt);

    // Parse property tree
    try
    {
        const ptree& ptc = pt.get_child("topology");

        for (const auto& v : ptc)
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
        return nullptr;
    }
    catch (ptree_bad_data& error)
    {
        cout << "ptree_bad_data: " << error.what() << endl;
        return nullptr;
    }
    catch (ptree_error& error)
    {
        cout << "ptree_error: " << error.what() << endl;
        return nullptr;
    }
    catch (out_of_range& error)
    {
        cout << "out_of_range: " << error.what() << endl;
        return nullptr;
    }
    catch (logic_error& error)
    {
        cout << "logic_error: " << error.what() << endl;
        return nullptr;
    }

    return m_main;
}

void DDSTopologyParserXML::ParsePort(const ptree& _pt)
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

void DDSTopologyParserXML::ParseTask(const ptree& _pt)
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

void DDSTopologyParserXML::ParseTaskCollection(const ptree& _pt)
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

void DDSTopologyParserXML::ParseTaskGroup(const ptree& _pt)
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

void DDSTopologyParserXML::ParseMain(const boost::property_tree::ptree& _pt)
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
                DDSTaskPtr_t newTask = make_shared<DDSTask>(*(task->second.get()));
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
                DDSTaskCollectionPtr_t newCollection = make_shared<DDSTaskCollection>(*(collection->second.get()));
                newCollection->setN(v.second.get<size_t>("<xmlattr>.n"));
                newCollection->setMinimumRequired(v.second.get<size_t>("<xmlattr>.minRequired"));
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
                DDSTaskGroupPtr_t newGroup = make_shared<DDSTaskGroup>(*(group->second.get()));
                newGroup->setN(v.second.get<size_t>("<xmlattr>.n"));
                newGroup->setMinimumRequired(v.second.get<size_t>("<xmlattr>.minRequired"));
                m_main->addElement(newGroup);
            }
            else
            {
                throw out_of_range(name + " task group does not exist.");
            }
        }
    }
}

void DDSTopologyParserXML::PrintPropertyTree(const string& _path, const ptree& _pt) const
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
