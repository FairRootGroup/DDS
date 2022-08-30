// Copyright 2016 GSI, Inc. All rights reserved.
//
//
//
//=============================================================================
// std
#include <fstream>
#include <iomanip>
#include <iostream>
// boost
#include <boost/tokenizer.hpp>
// Misc
#include "CustomIterator.h"
#include "MiscUtils.h"
#include "def.h"
// dds-ssh
#include "SSHConfigFile.h"
//=============================================================================
using namespace std;
using namespace dds;
using namespace dds::misc;
//=============================================================================
const char g_comment_char = '#';
const string g_bashscript_start = "@bash_begin@";
const string g_bashscript_end = "@bash_end@";
//=============================================================================
typedef boost::tokenizer<boost::escaped_list_separator<char>> Tok;
//=============================================================================

struct CSSHConfigFile::SImpl
{
    void read(const string& _filepath, bool _readBashOnly = false);
    void read(istream& _stream, bool _readBashOnly = false);

    static void make(const std::string& _filepath, const configRecords_t& _records, const std::string& _bash);
    static void make(std::ostream& _stream, const configRecords_t& _records, const std::string& _bash);
    static void make(const std::string& _filepath,
                     const std::vector<std::string>& _hosts,
                     const std::string& _sshOPtions,
                     const std::string& _wrkDir,
                     size_t _numSlots,
                     const std::string& _bash);
    static void make(std::ostream& _stream,
                     const std::vector<std::string>& _hosts,
                     const std::string& _sshOPtions,
                     const std::string& _wrkDir,
                     size_t _numSlots,
                     const std::string& _bash);

    configRecords_t m_records;
    std::string m_bash;
};

void CSSHConfigFile::SImpl::read(const string& _filepath, bool _readBashOnly)
{
    ifstream f(_filepath);
    if (!f.is_open())
    {
        stringstream ss;
        ss << "Failed to open dds-ssh configuration file " << std::quoted(_filepath);
        throw runtime_error(ss.str());
    }
    read(f, _readBashOnly);
}

void CSSHConfigFile::SImpl::read(istream& _stream, bool _readBashOnly)
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
            m_bash = "#! /usr/bin/env bash\n";
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
            m_bash += sLine;
            m_bash += "\n";
            continue;
        }

        if (!_readBashOnly)
        {
            Tok t(sLine);
            // create config. records here. But this class is not deleting them.
            // Each CWorker is responsible to delete it's config record info.
            configRecord_t rec{ make_shared<SConfigRecord>() };
            int res = rec->assign(t.begin(), t.end());
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
    }

    if (bCollectScript)
        throw runtime_error("dds-ssh configuration: syntax error. "
                            "There is a defined inline script, but the closing tag is missing.");
}

void CSSHConfigFile::SImpl::make(const std::string& _filepath,
                                 const configRecords_t& _records,
                                 const std::string& _bash)
{
    ofstream f(_filepath);
    if (!f.is_open())
    {
        stringstream ss;
        ss << "Failed to create dds-ssh configuration file " << std::quoted(_filepath);
        throw runtime_error(ss.str());
    }
    make(f, _records, _bash);
}

void CSSHConfigFile::SImpl::make(std::ostream& _stream, const configRecords_t& _records, const std::string& _bash)
{
    if (!_bash.empty())
    {
        _stream << g_bashscript_start << endl << _bash << endl << g_bashscript_end << endl << endl;
    }
    for (const auto& r : _records)
    {
        _stream << r->m_id << ", " << r->m_addr << ", " << r->m_sshOptions << ", " << r->m_wrkDir << ", " << r->m_nSlots
                << endl;
    }
}

void CSSHConfigFile::SImpl::make(const std::string& _filepath,
                                 const std::vector<std::string>& _hosts,
                                 const std::string& _sshOptions,
                                 const std::string& _wrkDir,
                                 size_t _numSlots,
                                 const std::string& _bash)
{
    ofstream f(_filepath);
    if (!f.is_open())
    {
        stringstream ss;
        ss << "Failed to create dds-ssh configuration file " << std::quoted(_filepath);
        throw runtime_error(ss.str());
    }
    make(f, _hosts, _sshOptions, _wrkDir, _numSlots, _bash);
}

void CSSHConfigFile::SImpl::make(std::ostream& _stream,
                                 const std::vector<std::string>& _hosts,
                                 const std::string& _sshOPtions,
                                 const std::string& _wrkDir,
                                 size_t _numSlots,
                                 const std::string& _bash)
{
    if (!_bash.empty())
    {
        _stream << g_bashscript_start << endl << _bash << endl << g_bashscript_end << endl << endl;
    }
    for (const auto& host : _hosts)
    {
        _stream << "wn_" << host << ", " << host << ", " << _sshOPtions << ", " << _wrkDir << ", " << _numSlots << endl;
    }
}

//=============================================================================

template <class InputIterator>
int SConfigRecord::assign(const InputIterator& _begin, const InputIterator& _end)
{
    InputIterator iter = _begin;
    if (iter == _end)
        return 1;
    m_id = *iter;
    trim(&m_id, ' ');

    if (++iter == _end)
        return 2;
    m_addr = *iter;
    trim(&m_addr, ' ');

    if (++iter == _end)
        return 3;
    m_sshOptions = *iter;
    trim(&m_sshOptions, ' ');

    if (++iter == _end)
        return 4;
    m_wrkDir = *iter;
    trim(&m_wrkDir, ' ');

    if (++iter == _end)
        return 5;
    if (!iter->empty())
    {
        std::stringstream ss;
        ss << *iter;
        ss >> m_nSlots;
    }

    return 0;
}

bool SConfigRecord::operator==(const SConfigRecord& _rec) const
{
    return (m_id == _rec.m_id && m_addr == _rec.m_addr && m_sshOptions == _rec.m_sshOptions &&
            m_wrkDir == _rec.m_wrkDir && m_nSlots == _rec.m_nSlots);
}

//=============================================================================

CSSHConfigFile::CSSHConfigFile(const std::string& _filepath)
    : m_impl(make_shared<CSSHConfigFile::SImpl>())
{
    m_impl->read(_filepath);
}

CSSHConfigFile::CSSHConfigFile(std::istream& _stream)
    : m_impl(make_shared<CSSHConfigFile::SImpl>())
{
    m_impl->read(_stream);
}

void CSSHConfigFile::make(const std::string& _filepath, const configRecords_t& _records, const std::string& _bash)
{
    SImpl::make(_filepath, _records, _bash);
}

void CSSHConfigFile::make(std::ostream& _stream, const configRecords_t& _records, const std::string& _bash)
{
    SImpl::make(_stream, _records, _bash);
}

void CSSHConfigFile::make(const std::string& _filepath,
                          const std::vector<std::string>& _hosts,
                          const std::string& _sshOPtions,
                          const std::string& _wrkDir,
                          size_t _numSlots,
                          const std::string& _bash)
{
    SImpl::make(_filepath, _hosts, _sshOPtions, _wrkDir, _numSlots, _bash);
}

void CSSHConfigFile::CSSHConfigFile::make(std::ostream& _stream,
                                          const std::vector<std::string>& _hosts,
                                          const std::string& _sshOPtions,
                                          const std::string& _wrkDir,
                                          size_t _numSlots,
                                          const std::string& _bash)
{
    SImpl::make(_stream, _hosts, _sshOPtions, _wrkDir, _numSlots, _bash);
}

const configRecords_t& CSSHConfigFile::getRecords()
{
    return m_impl->m_records;
}

const string& CSSHConfigFile::getBash()
{
    return m_impl->m_bash;
}
