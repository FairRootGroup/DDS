// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopologyParserXML.h"
#include "Task.h"
#include "TaskGroup.h"
#include "TaskCollection.h"
#include "UserDefaults.h"
#include "FindCfgFile.h"
#include "TopoVars.h"
// STL
#include <map>
// SYSTEM
#include <unistd.h>
#include <sys/wait.h>
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#pragma clang diagnostic pop

using namespace boost::property_tree;
using namespace std;
using namespace dds;

CTopologyParserXML::CTopologyParserXML()
{
}

CTopologyParserXML::~CTopologyParserXML()
{
}

bool CTopologyParserXML::isValid(const std::string& _fileName, bool _xmlValidationDisabled)
{
    if (_xmlValidationDisabled)
        return true;

    pid_t pid = fork();

    switch (pid)
    {
        case -1:
            // Unable to fork
            throw runtime_error("Unable to run XML validator.");
        case 0:
        {
            // FIXME: XSD file is hardcoded now -> take it from resource manager
            string topoXSDPath = CUserDefaults::getDDSPath() + "share/topology.xsd";
            MiscCommon::CFindCfgFile<string> cfg;
            cfg.SetOrder("/usr/bin/xmllint")("/usr/local/bin/xmllint")("/opt/local/bin/xmllint")("/bin/xmllint");
            string xmllintPath;
            cfg.GetCfg(&xmllintPath);

            execl(xmllintPath.c_str(), "xmllint", "--noout", "--schema", topoXSDPath.c_str(), _fileName.c_str(), NULL);

            // We shoud never come to this point of execution
            exit(1);
        }
    }

    int status = -1;
    while (wait(&status) != pid)
        ;

    return (status == 0);
}

void CTopologyParserXML::parse(const string& _fileName, TaskGroupPtr_t _main, bool _xmlValidationDisabled)
{
    if (_fileName.empty())
        throw runtime_error("topo file is not defined.");

    if (!boost::filesystem::exists(_fileName))
    {
        stringstream ss;
        ss << "Cannot locate the given topo file: " << _fileName;
        throw runtime_error(ss.str());
    }

    if (_main == nullptr)
        throw runtime_error("NULL input pointer.");

    try
    {
        // First we have to parse topology variables
        ptree varPT;
        read_xml(_fileName, varPT, xml_parser::no_comments);

        TopoVarsPtr_t vars = make_shared<CTopoVars>();
        vars->initFromPropertyTree("", varPT);

        // We have to replace all occurencies of topology variables in input XML file.
        stringstream ssPT;
        write_xml(ssPT, varPT);

        string strPT = ssPT.str();
        const CTopoVars::varMap_t& map = vars->getMap();
        for (const auto& v : map)
        {
            string varName = "${" + v.first + "}";
            boost::algorithm::replace_all(strPT, varName, v.second);
        }

        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        stringstream ssTmpFileName;
        ssTmpFileName << _fileName << "_" << uuid << ".xml";
        string tmpFileName = ssTmpFileName.str();

        {
            ofstream tmpFile(tmpFileName);
            tmpFile << strPT;
            tmpFile.close();
        }

        // Validate temporary XML file against XSD schema
        if (!isValid(tmpFileName, _xmlValidationDisabled))
            throw runtime_error("XML file is not valid.");

        ptree pt;
        read_xml(tmpFileName, pt);
        _main->initFromPropertyTree("main", pt);

        // Delete temporary file
        boost::filesystem::remove(tmpFileName);
    }
    catch (xml_parser_error& error)
    {
        throw runtime_error(string("Reading of input XML file failed with the following error: ") + error.what());
    }
    catch (exception& error) // ptree_error, out_of_range, logic_error
    {
        throw runtime_error(string("Initialization of Main failed with the following error") + error.what());
    }
}

void CTopologyParserXML::PrintPropertyTree(const string& _path, const ptree& _pt) const
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
