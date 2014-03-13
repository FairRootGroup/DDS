// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "DDSTopologyParserXML.h"
#include "DDSTask.h"
#include "DDSTaskGroup.h"
#include "DDSTaskCollection.h"
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

DDSTopologyParserXML::DDSTopologyParserXML() : m_main()
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
        m_main = make_shared<DDSTaskGroup>();
        m_main->initFromPropertyTree("main", pt);
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
