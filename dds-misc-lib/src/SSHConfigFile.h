// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef _DDS_NCF_H_
#define _DDS_NCF_H_
//
// - - - - - = = =     DDS NCF (nodes configuration file parcer)     = = = - - - - -
//
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
// STD
#include <vector>
namespace dds
{
    //=============================================================================
    /// \brief The class represents a single record of a dds-ssh configuration file
    struct SConfigRecord
    {
        template <class InputIterator>
        int assign(const InputIterator& _begin, const InputIterator& _end);
        bool operator==(const SConfigRecord& _rec) const;

        std::string m_id;
        std::string m_addr;
        std::string m_sshOptions;
        std::string m_wrkDir;
        size_t m_nSlots{ 1 };
    };
    //=============================================================================
    using configRecord_t = std::shared_ptr<SConfigRecord>;
    using configRecords_t = std::vector<configRecord_t>;
    //=============================================================================
    /// \brief Reads dds-ssh configuration file either from a text file or stream
    class CSSHConfigFile
    {
      public:
        CSSHConfigFile(const std::string& _filepath);
        CSSHConfigFile(std::istream& _stream);

        const configRecords_t& getRecords();
        const std::string& getBash();

      private:
        struct SImpl;
        std::shared_ptr<SImpl> m_impl;
    };
} // namespace dds
#endif
