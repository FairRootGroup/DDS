// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef CONFIG_H
#define CONFIG_H
// a configuration should be a comma-separated values (CSV) with
// the following records:
//
// id, login@host.fqdn, ssh params, remote working dir, number of workers,
//////// example:
// r1, anar@lxg0527.gsi.de, -p24, /tmp/test, 4
// r2, anar@lxi001.gsi.de,,/tmp/test,2
// 125, anar@lxg0055.gsi.de, -p22, /tmp/test,8
// ________________________________________________________
//
// it can be read from a stream.
// Fields are normally separated by commas. If you want to put a comma in a field,
// you need to put quotes around it. Also 3 escape sequences are supported.
//
//=============================================================================
// std
#include <vector>
#include <sstream>
// BOOST
#include <boost/shared_ptr.hpp>
// MiscCommon
#include "MiscUtils.h"
//=============================================================================
/// this class represents a single record of a dds-ssh configuration file
struct SConfigRecord
{
    SConfigRecord()
        : m_nWorkers(0)
    {
    }
    template <class InputIterator>
    int assignValues(const InputIterator& _begin, const InputIterator& _end)
    {
        InputIterator iter = _begin;
        if (iter == _end)
            return 1;
        m_id = *iter;
        MiscCommon::trim(&m_id, ' ');

        if (++iter == _end)
            return 2;
        m_addr = *iter;
        MiscCommon::trim(&m_addr, ' ');

        if (++iter == _end)
            return 3;
        m_sshOptions = *iter;
        MiscCommon::trim(&m_sshOptions, ' ');

        if (++iter == _end)
            return 4;
        m_wrkDir = *iter;
        MiscCommon::trim(&m_wrkDir, ' ');

        if (++iter == _end)
            return 5;
        if (!iter->empty())
        {
            std::stringstream ss;
            ss << *iter;
            ss >> m_nWorkers;
        }

        return 0;
    }
    bool operator==(const SConfigRecord& _rec) const
    {
        return (m_id == _rec.m_id && m_addr == _rec.m_addr && m_sshOptions == _rec.m_sshOptions &&
                m_wrkDir == _rec.m_wrkDir && m_nWorkers == _rec.m_nWorkers);
    }
    std::string m_id;
    std::string m_addr;
    std::string m_sshOptions;
    std::string m_wrkDir;
    size_t m_nWorkers;
};
//=============================================================================
typedef boost::shared_ptr<SConfigRecord> configRecord_t;
typedef std::vector<configRecord_t> configRecords_t;
//=============================================================================
class CConfig
{
  public:
    void readFrom(std::istream& _stream);
    configRecords_t getRecords();
    std::string getBashEnvCmds()
    {
        return m_bashEnvCmds;
    }

  private:
    configRecords_t m_records;
    std::string m_bashEnvCmds;
};
#endif
