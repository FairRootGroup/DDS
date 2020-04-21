// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoParserXML.h"
#include "Process.h"
#include "TopoGroup.h"
#include "TopoVars.h"
// BOOST
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#pragma clang diagnostic pop

using namespace std;
using namespace dds;
using namespace MiscCommon;
using namespace topology_api;
namespace fs = boost::filesystem;

void CTopoParserXML::parse(boost::property_tree::ptree& _pt,
                           const std::string& _filepath,
                           const std::string& _schemaFilepath,
                           std::string* _topologyName)
{
    if (_filepath.empty())
        throw std::runtime_error("topo file is not defined.");

    if (!fs::exists(_filepath))
    {
        std::stringstream ss;
        ss << "Cannot locate the given topo file: " << _filepath;
        throw std::runtime_error(ss.str());
    }

    try
    {
        // First we have to parse topology variables
        boost::property_tree::ptree varPT;
        read_xml(_filepath, varPT, boost::property_tree::xml_parser::no_comments);

        CTopoVars::Ptr_t vars{ CTopoBase::make<CTopoVars>("", varPT) };

        // We have to replace all occurencies of topology variables in input XML file.
        std::stringstream ssPT;
        write_xml(ssPT, varPT);

        std::string strPT = ssPT.str();
        const CTopoVars::varMap_t& map = vars->getMap();
        for (const auto& v : map)
        {
            std::string varName = "${" + v.first + "}";
            boost::algorithm::replace_all(strPT, varName, v.second);
        }

        fs::path tempDirPath = fs::temp_directory_path();
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        fs::path fsPath(_filepath);
        std::stringstream ssTmpFileName;
        ssTmpFileName << fsPath.filename().string() << "_" << uuid << ".xml";
        std::string tmpFilePath = tempDirPath.append(ssTmpFileName.str()).string();

        {
            std::ofstream tmpFile(tmpFilePath);
            tmpFile << strPT;
            tmpFile.close();
        }

        // Validate temporary XML file against XSD schema
        std::string output;
        if (!isValid(tmpFilePath, _schemaFilepath, &output))
            throw std::runtime_error(std::string("XML file is not valid. Error details: ") + output);

        read_xml(tmpFilePath, _pt);

        if (_topologyName != nullptr)
            *_topologyName = _pt.get_child("topology").get<std::string>("<xmlattr>.name");

        // Delete temporary file
        fs::remove(tmpFilePath);
    }
    catch (boost::property_tree::xml_parser_error& error)
    {
        throw std::runtime_error(std::string("Reading of input XML file failed with the following error: ") +
                                 error.what());
    }
    catch (std::exception& error) // ptree_error, out_of_range, logic_error
    {
        throw std::runtime_error(std::string("XML parsing failed with the following error: ") + error.what());
    }
}

bool CTopoParserXML::isValid(const std::string& _filepath, const std::string& _schemaFilepath, std::string* _output)
{
    if (_schemaFilepath.empty())
        return true;

    // Find command paths in $PATH
    boost::filesystem::path xmllintPath = bp::search_path("xmllint");
    // If we can't find xmllint throw exception with the proper error message
    if (xmllintPath.empty())
        throw runtime_error("Can't find xmllint. Use --disable-validation option in order to disable XML validation.");

    stringstream ssCmd;
    ssCmd << xmllintPath.string() << " --noout --schema "
          << "\"" << _schemaFilepath << "\""
          << " \"" << _filepath << "\"";

    string output;
    string errout;
    int exitCode;
    execute(ssCmd.str(), std::chrono::seconds(60), &output, &errout, &exitCode);

    if (_output != nullptr)
    {
        *_output = (output.empty()) ? "" : output;
        *_output += (errout.empty()) ? "" : (string("\n") + errout);
    }

    return (exitCode == 0);
}

void CTopoParserXML::PrintPropertyTree(const string& _path, const boost::property_tree::ptree& _pt)
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
