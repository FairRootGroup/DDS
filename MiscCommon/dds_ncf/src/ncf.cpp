// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
//=============================================================================
// std
#include <iostream>
// boost
#include <boost/tokenizer.hpp>
// MiscCommon
#include "CustomIterator.h"
#include "MiscUtils.h"
#include "def.h"
// dds-ssh
#include "ncf.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
using namespace dds;
using namespace dds::ncf;
//=============================================================================
const char g_comment_char = '#';
const string g_bashscript_start = "@bash_begin@";
const string g_bashscript_end = "@bash_end@";
//=============================================================================
typedef boost::tokenizer<boost::escaped_list_separator<char>> Tok;
//=============================================================================
void CNcf::readFrom(istream& _stream)
{
    // get lines from the configuration
    StringVector_t lines;
    copy(custom_istream_iterator<string>(_stream), custom_istream_iterator<string>(), back_inserter(lines));

    typedef set<string> ids_t;
    ids_t ids;

    // pars the configuration using boost's tokenizer
    StringVector_t::iterator iter = lines.begin();
    StringVector_t::const_iterator iter_end = lines.end();
    bool bCollectScript(false);
    for (size_t i = 0; iter != iter_end; ++iter, ++i)
    {
        // remove leading and trailing white space
        string sLine(*iter);
        trim<StringVector_t::value_type>(&sLine, ' ');

        // ignore empty lines
        if (sLine.empty())
            continue;

        // ignore comments only outside of inline bash script
        if (!bCollectScript && g_comment_char == sLine.at(0))
            continue;

        // check for bash script commands
        if (sLine.find(g_bashscript_start, 0) != StringVector_t::value_type::npos)
        {
            bCollectScript = true;
            sLine.erase(0, g_bashscript_start.size());
            m_bashEnvCmds = "#! /usr/bin/env bash\n";
            continue;
        }
        else if (sLine.find(g_bashscript_end, 0) != StringVector_t::value_type::npos)
        {
            bCollectScript = false;
            continue;
        }

        // Continue collecting the script
        if (bCollectScript)
        {
            m_bashEnvCmds += sLine;
            m_bashEnvCmds += "\n";
            continue;
        }

        Tok t(sLine);
        // create config. records here. But this class is not deleting them.
        // Each CWorker is responsible to delete it's config record info.
        configRecord_t rec = configRecord_t(new SConfigRecord());
        int res = rec->assignValues(t.begin(), t.end());
        if (res)
        {
            stringstream ss;
            ss << "dds-ssh configuration: syntax error at line " << i + 1;
            throw runtime_error(ss.str());
        }
        // check for duplicate ids
        pair<ids_t::iterator, bool> ret = ids.insert(rec->m_id);
        if (!ret.second)
        {
            stringstream ss;
            ss << "a not unique id has been found: "
               << "[" << rec->m_id << "]";
            throw runtime_error(ss.str());
        }

        // save a configuration record to a container
        m_records.push_back(rec);
    }

    if (bCollectScript)
        throw runtime_error("dds-ssh configuration: syntax error. "
                            "There is a defined inline script, but the closing tag is missing.");
}
//=============================================================================
configRecords_t CNcf::getRecords()
{
    return m_records;
}
