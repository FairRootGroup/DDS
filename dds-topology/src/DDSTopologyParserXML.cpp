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

DDSTopologyParserXML::DDSTopologyParserXML()
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

void DDSTopologyParserXML::parse(const string& _fileName, DDSTaskGroupPtr_t _main)
{
    // FIXME: Do we really need seperate try{} catch{} blocks?
    //        Or we have to put everything in one big try{} catch{} block in this function.

    if (_main == nullptr)
        throw runtime_error("NULL input pointer.");

    // First validate XML against XSD schema
    try
    {
        if (!isValid(_fileName))
            throw runtime_error("XML file is not valid.");
    }
    catch (runtime_error& error)
    {
        throw runtime_error(string("XML validation failed with the following error: ") + error.what());
    }

    ptree pt;

    // Read property tree from file
    try
    {
        read_xml(_fileName, pt);
    }
    catch (xml_parser_error& error)
    {
        throw runtime_error(string("Reading of input XML file failed with the following error: ") + error.what());
    }

    // Parse property tree
    try
    {
        _main->initFromPropertyTree("main", pt);
    }
    catch (exception& error) // ptree_error, out_of_range, logic_error
    {
        throw runtime_error(string("Initialization of Main failed with the following error") + error.what());
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
