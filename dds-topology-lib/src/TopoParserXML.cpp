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
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;
using namespace dds;
using namespace MiscCommon;
using namespace topology_api;
namespace fs = boost::filesystem;

void CTopoParserXML::parse(boost::property_tree::ptree& _pt,
                           const string& _filepath,
                           const string& _schemaFilepath,
                           string* _topologyName)
{
    if (_filepath.empty())
        throw runtime_error("Topo file is not defined.");

    if (!fs::exists(_filepath))
        throw runtime_error("Can't locate the given topo file: " + _filepath);

    ifstream stream(_filepath);
    if (stream.is_open())
    {
        parse(_pt, stream, _schemaFilepath, _topologyName);
    }
    else
    {
        throw runtime_error("Can't open te given topo file: " + _filepath);
    }
}

void CTopoParserXML::parse(boost::property_tree::ptree& _pt,
                           istream& _stream,
                           const string& _schemaFilepath,
                           string* _topologyName)
{
    try
    {
        // Parse topology variables
        boost::property_tree::ptree varPT;
        read_xml(_stream, varPT, boost::property_tree::xml_parser::no_comments);
        _stream.seekg(0);

        CTopoVars::Ptr_t vars{ CTopoBase::make<CTopoVars>("", varPT) };

        // Replace all occurencies of topology variables in input XML
        string strPT(istreambuf_iterator<char>(_stream), {});
        _stream.seekg(0);
        const CTopoVars::varMap_t& map{ vars->getMap() };
        for (const auto& v : map)
        {
            string varName{ "${" + v.first + "}" };
            boost::algorithm::replace_all(strPT, varName, v.second);
        }

        // Validate final XML against XSD schema
        istringstream ssPT{ strPT };
        string output;
        if (!isValid(ssPT, _schemaFilepath, &output))
            throw runtime_error(string("XML file is not valid. Error details: ") + output);

        // Read final XML in property tree
        read_xml(ssPT, _pt, boost::property_tree::xml_parser::no_comments);

        if (_topologyName != nullptr)
            *_topologyName = _pt.get_child("topology").get<string>("<xmlattr>.name");
    }
    catch (boost::property_tree::xml_parser_error& error)
    {
        throw runtime_error(string("Reading of input XML file failed with the following error: ") + error.what());
    }
    catch (exception& error) // ptree_error, out_of_range, logic_error
    {
        throw runtime_error(string("XML parsing failed with the following error: ") + error.what());
    }
}

bool CTopoParserXML::isValid(const string& _filepath, const string& _schemaFilepath, string* _output)
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
          << "\"" << _schemaFilepath << "\" " << _filepath;

    string output;
    string errout;
    int exitCode;
    execute(ssCmd.str(), chrono::seconds(60), &output, &errout, &exitCode);

    if (_output != nullptr)
    {
        *_output = (output.empty()) ? "" : output;
        *_output += (errout.empty()) ? "" : (string("\n") + errout);
    }

    return (exitCode == 0);
}

bool CTopoParserXML::isValid(istream& _stream, const string& _schemaFilepath, string* _output)
{
    try
    {
        // Serialize stream to a temporary file
        fs::path tmpFilePath{ fs::temp_directory_path() / fs::unique_path() };
        ofstream tmpStream(tmpFilePath.string());
        if (tmpStream.is_open())
        {
            tmpStream << _stream.rdbuf();
            tmpStream.close();
            _stream.seekg(0);
        }
        else
        {
            throw runtime_error("Can't open temp file " + tmpFilePath.string() + " for topology validation");
        }

        bool result{ isValid(tmpFilePath.string(), _schemaFilepath, _output) };

        fs::remove(tmpFilePath);

        return result;
    }
    catch (exception& _e)
    {
        throw runtime_error(_e.what());
    }
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
