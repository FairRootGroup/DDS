// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef WORKER_H
#define WORKER_H
//=============================================================================
// std
#include <iosfwd>
// MiscCommon
#include "def.h"
// pod-ssh
#include "local_types.h"
#include "ncf.h"
//=============================================================================
namespace dds
{
    namespace ssh_cmd
    {
        enum ETaskType
        {
            task_submit,
            task_clean,
            task_status,
            task_exec
        };
        //=============================================================================
        class CWorker
        {
          public:
            CWorker(ncf::configRecord_t _rec, const SWNOptions& _options, const std::string& _path);
            ~CWorker();

            void printInfo(std::ostream& _stream) const;
            bool run(ETaskType _param);
            std::string getID() const
            {
                return m_rec->m_id;
            }

          private:
            bool exec_command(const std::string& _cmd) const;
            void log(const std::string& _msg) const;

          private:
            ncf::configRecord_t m_rec;
            SWNOptions m_options;
            std::string m_path;
        };
    } // namespace ssh_cmd
} // namespace dds
#endif
