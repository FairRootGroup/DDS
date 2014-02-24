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

DDSTopology::DDSTopology() : m_tasks(), m_collections(), m_groups()
{
}

DDSTopology::~DDSTopology()
{
}

void DDSTopology::init(const std::string& _fileName)
{
    ptree pt;

    try
    {
        read_xml(_fileName, pt);
    }
    catch (xml_parser_error& error)
    {
        // FIXME: What to do in case of fail?
        std::cout << error.what();
    }

    PrintPropertyTree("", pt);

    ParsePropertyTree(pt);

    std::cout << toString();
}

void DDSTopology::ParsePropertyTree(const ptree& _pt)
{
    std::cout << "---ParsePropertyTree---" << std::endl;
    try
    {
        const ptree& pt = _pt.get_child("topology");

        std::string defaultGroup = pt.get<std::string>("<xmlattr>.default");

        for (auto it = pt.begin(); it != pt.end(); it++)
        {
            ptree::value_type v = *it;
            if (v.first == "task")
                ParsePropertyTreeTask(v.second);
            else if (v.first == "socket")
                ParsePropertyTreeSocket(v.second);
            else if (v.first == "collection")
                ParsePropertyTreeCollection(v.second);
            else if (v.first == "group")
                ParsePropertyTreeGroup(v.second);
        }
    }
    catch (ptree_bad_path& error)
    {
        std::cout << "ptree_bad_path: " << error.what() << std::endl;
    }
    catch (ptree_bad_data& error)
    {
        std::cout << "ptree_bad_data: " << error.what() << std::endl;
    }
    catch (ptree_error& error)
    {
        std::cout << "ptree_error: " << error.what() << std::endl;
    }
}

void DDSTopology::ParsePropertyTreeTask(const ptree& _pt)
{
    std::string name = _pt.get<std::string>("<xmlattr>.name");
    std::string exec = _pt.get<std::string>("<xmlattr>.exec");
    std::string socket = _pt.get<std::string>("<xmlattr>.socket");

    std::cout << "task: " << name << " " << exec << " " << socket << std::endl;
    // FIXME: create task object and add to array of tasks
    DDSTask task;
    task.setName(name);
    task.setExec(exec);
    task.setSockets({socket});
    m_tasks.push_back(task);
}

void DDSTopology::ParsePropertyTreeSocket(const ptree& _pt)
{
    std::string name = _pt.get<std::string>("<xmlattr>.name");
    unsigned int min = _pt.get<unsigned int>("<xmlattr>.min");
    unsigned int max = _pt.get<unsigned int>("<xmlattr>.max");

    std::cout << "socket: " << name << " " << min << " " << max << std::endl;
    // FIXME: create socket object and add it to array
}

void DDSTopology::ParsePropertyTreeCollection(const ptree& _pt)
{
    std::string name = _pt.get<std::string>("<xmlattr>.name");
    std::vector<std::string> tasks;
    for (auto it = _pt.begin(); it != _pt.end(); it++)
    {
        ptree::value_type v = *it;
        if (v.first == "task")
        {
            std::string name = v.second.get<std::string>("<xmlattr>.name");
            tasks.push_back(name);
        }
    }

    std::cout << "collection: " << name << " ";
    for_each(tasks.begin(),
             tasks.end(),
             [](const std::string& _v)
    { std::cout << _v << " "; });
    std::cout << std::endl;
    // FIXME: create collection object and add it to array
    DDSTaskCollection collection;
    collection.setName(name);
    collection.setTasks(tasks);
    m_collections.push_back(collection);
}

void DDSTopology::ParsePropertyTreeGroup(const ptree& _pt)
{
    std::string name = _pt.get<std::string>("<xmlattr>.name");
    std::vector<std::string> collections;
    for (auto it = _pt.begin(); it != _pt.end(); it++)
    {
        ptree::value_type v = *it;
        if (v.first == "collection")
        {
            std::string name = v.second.get<std::string>("<xmlattr>.name");
            collections.push_back(name);
        }
    }

    std::cout << "group: " << name << " ";
    for_each(collections.begin(),
             collections.end(),
             [](const std::string& _v)
    { std::cout << _v << " "; });
    std::cout << std::endl;
    // FIXME: create group object nd add it to array
    DDSTaskGroup group;
    group.setName(name);
    group.setTaskCollections(collections);
    m_groups.push_back(group);
}

void DDSTopology::PrintPropertyTree(const std::string& _path,
                                    const ptree& _pt) const
{
    if (_pt.size() == 0)
    {
        std::cout << _path << " " << _pt.get_value("") << std::endl;
        return;
    }
    for (auto it = _pt.begin(); it != _pt.end(); it++)
    {
        boost::property_tree::ptree::value_type v = *it;
        std::string path = (_path != "") ? (_path + "." + v.first) : v.first;
        PrintPropertyTree(path, v.second);
    }
}
